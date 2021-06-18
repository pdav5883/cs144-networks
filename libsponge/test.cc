#include <iostream>
#include <list>
#include <string>
#include "stream_reassembler.hh"
#include "byte_stream.hh"
#include "wrapping_integers.hh"

using namespace std;

int main() {

    /*
    uint64_t ref = 0x3ffffffff + 1;
    uint64_t cp = ref - 1000000;
    WrappingInt32 isn(11);
    WrappingInt32 n(10);
    uint64_t un_fun = unwrap(n, isn, cp);
    uint64_t un_truth = ref - 1; 
    cout << "truth: " << un_truth << endl;
    cout << "fun: " << un_fun << endl;
    cout << "err: " << un_truth - un_fun << endl;
    */
    //cout << wrap(3 * (1ll << 32), WrappingInt32(0)) << endl; // 0
    //cout << wrap(3 * (1ll << 32) + 17, WrappingInt32(15)) << endl; // 32
    cout << "max u32: " << 0xffffffff << ", half max u32: " << 0x7fffffff << endl;
    cout << unwrap(WrappingInt32(0xffffffff), WrappingInt32(0), 0) << ", " << 0xffffffff << endl; // 13
    cout << unwrap(WrappingInt32(0), WrappingInt32(0), 0) << ", " << 0 << endl; // 13
    cout << unwrap(WrappingInt32(0xffffffff - 0xff843), WrappingInt32(0), 0) << ", " << 0xffffffff - 0xff843 << endl; // 13
}

