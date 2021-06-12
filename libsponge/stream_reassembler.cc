#include "stream_reassembler.hh"
#include <iostream>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

string list_to_string(const list<byte> &l) {
    return string(l.begin(), l.end());
}

list<byte> string_to_list(const string &s) {
    return list<byte>(s.begin(), s.end());
}


void StreamReassembler::merge_bytesegments() {
    return;
}


void StreamReassembler::update_stream() {
    // only do something if we have bytes ready to write to stream
    if (empty() || buffer.front().firstindex != nextindex) {
        return;
    }

    // the first element of buffer is ready to add to the output stream,
    // take it off the buffer, add string to stream, increment next ind
    elem front = buffer.front();
    buffer.pop_front();
    _output.write(list_to_string(front.bytesegment));
    nextindex += front.numbytes;

    // end the stream if we've written all bytes and have valid eof
    if (eofready || nextindex > lastindex) {
        _output.end_input();
    }
}


size_t StreamReassembler::total_width() const {
    if (empty()) {
        return _output.buffer_size();
    }
    else {
        return _output.buffer_size() + buffer.back().firstindex + buffer.back().numbytes - nextindex;
    }
}


StreamReassembler::StreamReassembler(const size_t capacity) 
    : _output(capacity),
    _capacity(capacity),
    nextindex(0),
    buffer(list<elem>()),
    lastindex(0),
    eofready(false) {}


void StreamReassembler::printall() {
    cout << "-----" << endl;
    cout << "Total Width: " << total_width() << endl;
    cout << "Stream: " << _output.peek_output(_output.buffer_size()) << endl;
    cout << "Next Ind: " << nextindex << endl;
    cout << "Buffer: " << endl;
    for (elem e : buffer) {
        cout << "  { " << e.firstindex << ", " << e.numbytes << ", ( ";
        for (byte b : e.bytesegment) {
            cout << b << " ";
        }
        cout << ") }" << endl;
    }
}


//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const uint64_t index, const bool eof) {
    list<elem>::iterator iter = buffer.begin();
    uint64_t prevseg_post = nextindex;
    uint64_t startsub = index;
    uint64_t endsub = index + data.length() - 1;

    uint64_t startgap, endgap, startind, endind;

    while (iter != buffer.end()) {
        startgap = prevseg_post;
        endgap = iter->firstindex - 1;

        startind = max(startgap, startsub);
        endind = min(endgap, endsub);

        // insert the substring in this gap, then advance the iterator once
        if (startind <= endind) {
            size_t n = endind - startind + 1;
            string s = data.substr(startind - index, n);
            buffer.insert(iter, {startind, n, string_to_list(s)});
            iter++;
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
        buffer.insert(iter, {startind, n, string_to_list(s)});
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


bool StreamReassembler::empty() const {
    return buffer.empty();
}

