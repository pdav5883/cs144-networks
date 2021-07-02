#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (!_syn) {
        if (seg.header().syn) {
            _syn = true;
            _isn = seg.header().seqno;
        } else {
            return;
        }
    }

    // construct the index of the payload, -1 goes from abs seqno to stream index
    uint64_t index = unwrap(seg.header().seqno, _isn, _nextind) - 1;

    // remember that syn takes up an seqno if it is sent
    if (seg.header().syn) {
        index++;
    }

    // push the bytes to reassembler, move nextind based on stream size
    size_t stream_size_before = stream_out().buffer_size();
    _reassembler.push_substring(seg.payload().copy(), index, seg.header().fin);  // has to be a better way than copy()
    _nextind += stream_out().buffer_size() - stream_size_before;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_syn) {
        return {};
    } else if (stream_out().input_ended()) {
        return wrap(_nextind + 2, _isn);  // account for the fin bit
    } else {
        return wrap(_nextind + 1, _isn);
    }
}

size_t TCPReceiver::window_size() const { return _capacity - stream_out().buffer_size(); }
