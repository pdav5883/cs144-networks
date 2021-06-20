#include "byte_stream.hh"
#include "stream_reassembler.hh"
#include "tcp_header.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <iostream>
#include <list>
#include <string>

using namespace std;
/*
TCPSegment buildseg(string data, WrappingInt32 seqno, bool syn, bool fin) {
    TCPSegment seg;
    seg.header().syn = syn;
    seg.header().fin= fin;
    seg.header().seqno = seqno;
    seg.payload() = data;
    return seg;
}
*/

int main() {
    // cout << buildseg("hello", WrappingInt32(23), true, false) << endl;
    return 0;
}
