#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

// TODO: why need to return bool?
bool TCPConnection::_send_outgoing(bool force_send) {
    if (_sender.segments_out().empty()) {
        if (force_send) {
            //cout << "XX: FORCE SEND" << endl;
            _sender.send_empty_segment();
            //_sender.fill_window();
        }
        else {
            return false;
        }
    }

    //cout << "XX: SENDING STUFF....." << endl;

    // create segments and push them to connection queue
    while (!_sender.segments_out().empty()) {
        TCPSegment seg = _sender.segments_out().front();
        seg.header().ack = _receiver.ackno().has_value();

        if (seg.header().ack) {
            seg.header().ackno = _receiver.ackno().value();
            seg.header().win = min<uint16_t>(_receiver.window_size(), numeric_limits<uint16_t>::max());
        }
        _segments_out.push(seg);
        //cout << "XX: SEND: " << seg.length_in_sequence_space() << endl;
        _sender.segments_out().pop();
    }
    return true;
}

bool TCPConnection::_send_outgoing() {
    return _send_outgoing(false);
}

void TCPConnection::_send_rst() {
    if (_sender.segments_out().empty()) {
        _sender.send_empty_segment();
    }
    
    _sender.segments_out().front().header().rst = true;
    _send_outgoing();

    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _active = false;
}

void TCPConnection::_check_shutdown() {
    // received fin
    bool req1 = _receiver.stream_out().eof();
    
    // sent fin - TODO has to be better way. need second part because could be no window space to send fin
    bool req2 = _sender.stream_in().eof() && _sender.next_seqno_absolute() >= 2 + _sender.stream_in().bytes_read(); 
    
    // sent fin is acked (at least everything that's been sent is acked)
    bool req3 = _sender.bytes_in_flight() == 0;

    // whether we are good for passive close
    if (req1 && !req2) {
        _linger_after_streams_finish = false;        
    }

    // confident that peer got our ack of its fin
    if (req1 && req2 && req3) {
        if (!_linger_after_streams_finish || _timer_received >= 10 * _cfg.rt_timeout) {
            _active = false;
        }
    }
}

size_t TCPConnection::remaining_outbound_capacity() const {
    return _sender.stream_in().remaining_capacity();    
}

size_t TCPConnection::bytes_in_flight() const {
    return _sender.bytes_in_flight();
}

size_t TCPConnection::unassembled_bytes() const {
    return _receiver.unassembled_bytes();
}

size_t TCPConnection::time_since_last_segment_received() const {
    return _timer_received;
}

void TCPConnection::segment_received(const TCPSegment &seg) {
    _timer_received = 0;

    if (seg.header().rst) {
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _active = false;
        return;
    }
    
    //cout << "XX: RECEIVED STUFF:" << seg.length_in_sequence_space() << endl;
    
    _receiver.segment_received(seg);

    // attempt to connect if someone is trying to connect with us with a syn
    if (!_connected && seg.header().syn) {
        connect();
        return;
    }

    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }

    _send_outgoing(seg.length_in_sequence_space() > 0); // need to force here if there is something to ack 
    _check_shutdown();
}

bool TCPConnection::active() const {
    return _active;
}

size_t TCPConnection::write(const string &data) {
    //cout << "XX: CALL WRITE:";
    size_t written_bytes = _sender.stream_in().write(data);
    _sender.fill_window();
    _send_outgoing();
    //cout << written_bytes << endl;
    return written_bytes;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _timer_received += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);

    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        _send_rst();
        return;
    }

    _send_outgoing();
    _check_shutdown();
}

void TCPConnection::end_input_stream() {
    //cout << "XX: CALL END" << endl;
    //cout << "XX: BUFFER SIZE: " << _sender.stream_in().buffer_size() << endl;
    _sender.stream_in().end_input();
    _sender.fill_window();
    _send_outgoing();
}

void TCPConnection::connect() {
    _connected = true;
    _sender.fill_window();
    _send_outgoing();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // need to send a RST segment to the peer
            _send_rst();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
