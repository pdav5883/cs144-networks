#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : maxlen(capacity)
    , buffer(string(maxlen, ' '))
    , sind(-1)
    , eind(-1)
    , cnt_read(0)
    , cnt_write(0)
    , _error(false)
    , _input_active(true) {}

size_t ByteStream::write(const string &data) {
    size_t writenum = min(data.length(), remaining_capacity());
    //cout << "data length: " << data.length() << "...rem cap: " << remaining_capacity() << "...writenum: " << writenum << endl;

    // empty data case
    if (writenum == 0) {
        return 0;
    }

    // buffer empty case
    if (buffer_empty()) {
        sind = 0;
        eind = -1;
    }

    for (size_t i = 0; i < writenum; i++) {
        eind = (eind + 1) % maxlen;
        buffer[eind] = data[i];
    }

    cnt_write += writenum;
    return writenum;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t readnum = min(buffer_size(), len);
    string out = string(readnum, ' ');

    for (size_t i = 0; i < readnum; i++) {
        out[i] = buffer[(sind + i) % maxlen];
    }
    return out;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    if (len >= buffer_size()) {
        cnt_read += buffer_size();
        sind = -1;

    } else {
        cnt_read += len;
        sind = (sind + len) % maxlen;
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    //cout << "======READ======" << endl;
    string out = peek_output(len);
    pop_output(len);
    return out;
}

void ByteStream::end_input() { _input_active = false; }

bool ByteStream::input_ended() const { return !_input_active; }

size_t ByteStream::buffer_size() const {
    if (buffer_empty()) {
        return 0;
    }
    // c++ modulus doesn't behave like python with neg nums
    else {
        return (maxlen + (eind - sind) % maxlen) % maxlen + 1;
    }
}

bool ByteStream::buffer_empty() const { return (sind == -1); }

bool ByteStream::eof() const { return !_input_active && buffer_empty(); }

size_t ByteStream::bytes_written() const { return cnt_write; }

size_t ByteStream::bytes_read() const { return cnt_read; }

size_t ByteStream::remaining_capacity() const {
    if (input_ended()) {
        return 0;
    }
    else {
        return maxlen - buffer_size();
    }
}

