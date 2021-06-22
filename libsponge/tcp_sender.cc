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
void _send_segment(TCPSegment &seg) {
    _segments_out.push(seg);
    _segments_unack.push(seg);
    _timer = 0;
}

void _resend_segment() {
    _segments_out.push(_segments_unack.front());
    _timer = 0;
}

void _update_unack() {
    TCPSegment seg;
    uint64_t seg_end_seqno;

    // if ackno is past the end of the unacknowledged segment, pop it
    while (!_segments_unack.empty()) {
        seg = _segments_unack.front();
        seg_end_seqno = unwrap(seg.header().seqno, _isn, _prev_ackno)
                                 + seg.length_in_sequence_space() - 1;

        if (_prev_ackno > seg_end_seqno) {
            _segments_unack.pop();
        }
        else {
            break;
        }
    }
}

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _current_tetransmission_timeout{retx_timeout} {}

uint64_t TCPSender::bytes_in_flight() const { return {}; }

void TCPSender::fill_window() {}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t curr_ackno = unwrap(ackno, _isn, _prev_ackno);

    // only do something if the ackno is higher than what we already have
    if (curr_ackno > _prev_ackno) {
        _prev_ackno = curr_ackno;
        _receiver_window = window_size;
        
        _current_retransmission_timeout = _initial_retransmission_timeout;
        _retry_count = 0;
        _timer = 0;

        _update_unack();
    }
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

void TCPSender::send_empty_segment() {}
