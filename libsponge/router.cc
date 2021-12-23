#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";

    table_item item = {route_prefix, prefix_length, next_hop, interface_num};
    _table.push_back(item);
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    // drop if ttl expired or about to expire
    if (dgram.header().ttl <= 1) {
        return;
    }

    uint32_t dgram_dst = dgram.header().dst;

    list<table_item>::iterator it = _table.begin();
    list<table_item>::iterator longest_match = _table.end();
    uint8_t longest_length = 0;

    // look through table and find longest prefix match
    while (it != _table.end()) {
        uint8_t shiftnum = ADDRESS_BITS - it->prefix_length;
        bool match = (it->route_prefix >> shiftnum) == (myaddr_numeric >> shiftnum) || shiftnum == ADDRESS_BITS;
        if (match && it->prefix_length >= longest_length) {
            longest_match = it;
            longest_length = it->prefix_length;
        }
        it++;
    }

    // drop if we didn't find a match
    if (longest_match == _table.end()) {
        return;
    }
    // send onward if we found match
    else {
        Address route_dst;

        if (longest_match->next_hop.has_value()) {
            route_dst = next_hop;
        }
        else {
            route_dst = Address::from_ipv4_numeric(dgram_dst);
        }

        dgram.header().ttl--;
        interface(longest_match->interface_num).send_datagram(dgram, route_dst);
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
