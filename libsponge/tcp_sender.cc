#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;


// Helper methods
void TCPSender::_send_segment(const TCPSegment seg) {
    _segments_out.push(seg);
    _segments_unack.push(seg);
    _bytes_unack += seg.length_in_sequence_space();
    _timer = 0;
}

void TCPSender::_resend_segment() {
    _segments_out.push(_segments_unack.front());
    _timer = 0;
}

void TCPSender::_update_unack() {
    TCPSegment seg;
    uint64_t seg_end_seqno;

    // if ackno is past the end of the unacknowledged segment, pop it
    while (!_segments_unack.empty()) {
        seg = _segments_unack.front();
        seg_end_seqno = unwrap(seg.header().seqno, _isn, _prev_ackno)
                                 + seg.length_in_sequence_space() - 1;

        if (_prev_ackno > seg_end_seqno) {
            _bytes_unack -= seg.length_in_sequence_space();
            _segments_unack.pop();
        }
        else {
            break;
        }
    }
}

const TCPSegment TCPSender::_buildseg(string data, const WrappingInt32 seqno, bool syn, bool fin) {
    TCPSegment seg;
    seg.header().syn = syn;
    seg.header().fin= fin;
    seg.header().seqno = seqno;
    seg.payload() = Buffer(move(data));
    return seg;
}

bool TCPSender::_has_content() {
    // return whether there are any bytes to put in the segment: syn or fyn or payload
    return next_seqno_absolute() == 0 || stream_in().eof() || stream_in().buffer_size() > 0;
}

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _current_retransmission_timeout{retx_timeout} {}

uint64_t TCPSender::bytes_in_flight() const {
    /*uint64_t bif = 0;

    for (TCPSegment seg : _segments_unack) {
        bif += seg.length_in_sequence_space();
    }

    return bif;*/
    return _bytes_unack;
}

void TCPSender::fill_window() {
    // how much space in window, per instructions window is always at least one
    uint16_t one = 1; // TODO do something about this
    uint64_t window  = max(_receiver_window, one);
    uint64_t unsent_window = window - bytes_in_flight();
    uint64_t seg_bytes = 0;
    bool send_syn = false;
    bool send_fin = false;

    // haven't sent syn
    // haven't sent

    while (unsent_window > 0 && !_sent_fin && _has_content()) {
        // see if segment will contain syn
        if (next_seqno_absolute() == 0) {
            send_syn = true;
            unsent_window--;
            seg_bytes++;
        }

        // payload bytes constrained by receiver window, max payload size, input buffer
        uint16_t payload_bytes = min(unsent_window, min(TCPConfig::MAX_PAYLOAD_SIZE, stream_in().buffer_size()));
        string payload = stream_in().read(payload_bytes); 
        unsent_window -= payload_bytes;
        seg_bytes += payload_bytes;

        // check whether to set fin flag
        if (stream_in().eof() && unsent_window > 0) {
            send_fin = true;
            _sent_fin = true;
            unsent_window--;
            seg_bytes++;
        }
        _send_segment(_buildseg(payload, next_seqno(), send_syn, send_fin));
        _next_seqno += seg_bytes;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t curr_ackno = unwrap(ackno, _isn, _prev_ackno);

    // only do something if the ackno is higher than what we already have
    if (curr_ackno > _prev_ackno) {
        _prev_ackno = curr_ackno;
        _receiver_window = window_size; // TODO: maybe need to adjust window if curr_ackno == _prev_ackno as well
        
        _current_retransmission_timeout = _initial_retransmission_timeout;
        _retry_count = 0;
        _timer = 0;

        _update_unack();
    }
    fill_window(); // fill the window even if we didn't ack anything new, in case there's new data avail on sender side
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_segments_unack.empty()) {
        _timer += ms_since_last_tick;
    }

    if (_timer >= _current_retransmission_timeout) {
        _resend_segment();
    
        if (_receiver_window > 0) {
            _retry_count++;
            _current_retransmission_timeout *= 2;
        }
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _retry_count; }

void TCPSender::send_empty_segment() {
    _segments_out.push(_buildseg("", next_seqno(), false, false));
}
