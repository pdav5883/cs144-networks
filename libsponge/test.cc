#include "byte_stream.hh"
#include "stream_reassembler.hh"
#include "tcp_header.hh"
#include "tcp_segment.hh"
#include "buffer.hh"
#include "wrapping_integers.hh"

#include <iostream>
#include <list>
#include <string>

using namespace std;

TCPSegment buildseg(string data, const WrappingInt32 seqno, bool syn, bool fin) {
    TCPSegment seg;
    seg.header().syn = syn;
    seg.header().fin= fin;
    seg.header().seqno = seqno;
    seg.payload() = Buffer(move(data));
    return seg;
}


int main() {
    const string s = "this is a string";
    TCPSegment seg = buildseg(s, WrappingInt32(44), true, true);
    int i = 10;
    cout << seg.payload().str() << endl;
    return 0;
}
