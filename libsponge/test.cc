#include "buffer.hh"
#include "byte_stream.hh"
#include "stream_reassembler.hh"
#include "tcp_header.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <iostream>
#include <list>
#include <sstream>
#include <string>

using namespace std;

TCPSegment buildseg(string data, const WrappingInt32 seqno, bool syn, bool fin) {
    TCPSegment seg;
    seg.header().syn = syn;
    seg.header().fin = fin;
    seg.header().seqno = seqno;
    seg.payload() = Buffer(move(data));
    return seg;
}
/*
string TCPHeader::to_string() const {
    stringstream ss{};
    ss << hex << boolalpha << "TCP source port: " << +sport << '\n'
       << "TCP dest port: " << +dport << '\n'
       << "TCP seqno: " << seqno << '\n'
       << "TCP ackno: " << ackno << '\n'
       << "TCP doff: " << +doff << '\n'
       << "Flags: urg: " << urg << " ack: " << ack << " psh: " << psh << " rst: " << rst << " syn: " << syn
       << " fin: " << fin << '\n'
       << "TCP winsize: " << +win << '\n'
       << "TCP cksum: " << +cksum << '\n'
       << "TCP uptr: " << +uptr << '\n';
    return ss.str();
}


size_t TCPSegment::length_in_sequence_space() const {
    return payload().str().size() + (header().syn ? 1 : 0) + (header().fin ? 1 : 0);
}

int main() {
    const string s = "this is a string";
    TCPSegment seg1 = buildseg("", WrappingInt32(44), false, false);
    cout << "------------" << endl;
    cout << "len: " << seg1.length_in_sequence_space() << endl << endl;
    cout << "header: " << seg1.header().to_string() << endl;
    cout << "payload: " << seg1.payload().str() << endl;
    return 0;
}*/
