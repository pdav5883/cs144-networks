#include <iostream>
#include <list>
#include <string>
#include "stream_reassembler.hh"
#include "byte_stream.hh"

using namespace std;

/*typedef unsigned char byte;
typedef struct elem {
    int index;
    list<byte> segment;
} elem;/

void printelem(elem &e) {
    cout << e.index << ":{ ";

    for (byte i : e.segment) {
        cout << i << " ";
    }
    cout << "}" << endl;
}

void printq(list<elem> &q) {
    cout << "-----" << endl;
    for (elem e : q) {
        printelem(e);
    }
    cout << "-----" << endl;
}


string list_to_string(const list<byte> &l) {
    return string(l.begin(), l.end());
}

list<byte> string_to_list(const string &s) {
    return list<byte>(s.begin(), s.end());
}
*/


int main() {
    /*
    list<elem> q = list<elem>();
    q.push_front({4, {'a','b','z'}});
    q.push_back({5, {'c','d'}});
    printq(q);
    elem e = q.back();
    list<byte> &l1 = q.front().segment;
    list<byte> &l2 = e.segment;
    l1.splice(l1.end(), l2);
    q.pop_back();
    printelem(e);
    printq(q);
    string s = "hello";
    elem e = {6, string_to_list(s)};
    list<byte> l = string_to_list(s);
    l.push_back('!');
    string s2 = list_to_string(l);
    printelem(e);
    cout << s2 << endl;
    return 0;
    */
    StreamReassembler sr(8);
    ByteStream &bs = sr.stream_out();
    sr.printall();
    sr.push_substring("23456",2,false);
    sr.printall();
    sr.push_substring("0123456789",0,false);
    sr.printall();
    cout << "Read: " << bs.read(4) << endl;
    sr.push_substring("89",8,false);
    sr.printall();
}

