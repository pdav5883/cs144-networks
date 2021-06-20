#include "stream_reassembler.hh"

#include <iostream>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void StreamReassembler::merge_bytesegments() {
    if (empty()) {
        return;
    }

    list<elem>::iterator left = buffer.begin();
    list<elem>::iterator right = next(left);

    while (right != buffer.end()) {
        if (left->firstindex + left->numbytes == right->firstindex) {
            // merge right segment into left segment, remove right elem, reset right elem
            left->bytesegment += right->bytesegment;
            left->numbytes += right->numbytes;
            right = buffer.erase(right);
        } else {
            left = next(left);
            right = next(right);
        }
    }
}

void StreamReassembler::update_stream() {
    // only need to do something if we have bytes ready to write to stream
    if (!empty() && buffer.front().firstindex == nextindex) {
        // the first element of buffer is ready to add to the output stream,
        // take it off the buffer, add string to stream, increment next ind
        elem front = buffer.front();
        buffer.pop_front();
        _output.write(front.bytesegment);
        nextindex += front.numbytes;
    }

    // end the stream if we've written all bytes and have valid eof
    if (eofready && nextindex >= eofindex) {
        _output.end_input();
    }
}

size_t StreamReassembler::total_width() const {
    if (empty()) {
        return _output.buffer_size();
    } else {
        return _output.buffer_size() + buffer.back().firstindex + buffer.back().numbytes - nextindex;
    }
}

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), nextindex(0), buffer(list<elem>()), eofindex(0), eofready(false) {}

void StreamReassembler::printall() {
    cout << "-----" << endl;
    cout << "Total Width: " << total_width() << endl;
    cout << "Stream: " << _output.peek_output(_output.buffer_size()) << endl;
    cout << "Next Ind: " << nextindex << endl;
    cout << "Buffer: " << endl;
    for (elem e : buffer) {
        cout << "  { " << e.firstindex << ", " << e.numbytes << ", (" << e.bytesegment << ") }" << endl;
    }
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const uint64_t index, const bool eof) {
    // deal with eof flag and/or no data
    if (eof) {
        eofready = true;
        eofindex = index + data.length();
    }

    if (data.length() == 0) {
        update_stream();  // need to do this here in case eof flag was passed with no data
        return;
    }

    list<elem>::iterator iter = buffer.begin();
    uint64_t prevseg_post = nextindex;
    uint64_t startsub = index;

    // enforces the max capacity limit of the stream + buffer
    uint64_t endsub = min(index + data.length() - 1, nextindex + _capacity - _output.buffer_size() - 1);

    uint64_t startgap, endgap, startind, endind;

    while (iter != buffer.end()) {
        startgap = prevseg_post;
        endgap = iter->firstindex - 1;

        if (startgap > endsub) {
            break;
        }

        startind = max(startgap, startsub);
        endind = min(endgap, endsub);

        // insert the substring in this gap, then advance the iterator once
        if (startind <= endind) {
            size_t n = endind - startind + 1;
            string s = data.substr(startind - index, n);
            buffer.insert(iter, {startind, n, s});
        }

        prevseg_post = iter->firstindex + iter->numbytes;
        iter++;
    }

    // need one more to insert between the last existing segment and the end of the buffer
    startgap = prevseg_post;
    endgap = nextindex + _capacity - _output.buffer_size() - 1;

    startind = max(startgap, startsub);
    endind = min(endgap, endsub);

    // insert the substring in this gap, then advance the iterator once
    if (startind <= endind) {
        size_t n = endind - startind + 1;
        string s = data.substr(startind - index, n);
        buffer.insert(iter, {startind, n, s});
    }

    merge_bytesegments();
    update_stream();
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t n = 0;
    for (elem e : buffer) {
        n += e.numbytes;
    }
    return n;
}

bool StreamReassembler::empty() const { return buffer.empty(); }
