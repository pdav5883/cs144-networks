#include "address.hh"
#include <list>
#include <iostream>
#include <optional>

using namespace std;

typedef struct table_item {
    const uint32_t route_prefix;
    const uint8_t prefix_length;
    const optional<Address> next_hop;
    const size_t interface_num;
} table_item;

int main() {
    list<table_item> _table;
    
    Address addr1("120.120.0.0",0);
    Address addr2("120.120.4.0",0);
    Address addr3("120.0.0.0",0);
    Address addr4("0.0.0.0",0);

    table_item i1 = {addr1.ipv4_numeric(), 16, {}, 0};
    table_item i2 = {addr2.ipv4_numeric(), 24, {}, 1};
    table_item i3 = {addr3.ipv4_numeric(), 8, {}, 2};
    table_item i4 = {addr4.ipv4_numeric(), 0, {}, 3};

    _table.push_back(i1);
    _table.push_back(i2);
    _table.push_back(i3);
    _table.push_back(i4);

    Address myaddr("10.20.5.26",0);
    uint32_t myaddr_numeric = myaddr.ipv4_numeric();

    list<table_item>::iterator it = _table.begin();
    list<table_item>::iterator longest_match = _table.end();
    uint8_t longest_length = 0;

    while (it != _table.end()) {
        uint8_t shiftnum = 32 - it->prefix_length;
        bool match = (it->route_prefix >> shiftnum) == (myaddr_numeric >> shiftnum) || shiftnum == 32;
        if (match && it->prefix_length >= longest_length) {
            longest_match = it;
            longest_length = it->prefix_length;
        }
        it++;
    }
    if (longest_match == _table.end()) {
        cout << "match not found" << endl;
    }
    else {
        cout << "match port " << longest_match->interface_num << endl;
    }

    //Address addr("121.12.0.23",5);
    //cout << "Numeric address " << addr.ipv4_numeric() << endl;
    //cout << "Table length " << _table.size() << endl;
    //cout << "Test successful!" << endl;
    return 0;
}
