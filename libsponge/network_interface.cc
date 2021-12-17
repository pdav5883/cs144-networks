include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    // cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address " << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    // check if next_hop_ip is in cache
    map<uint32_t, cache_value>::iterator it = _arp_cache.find(next_hop_ip);
    bool send_arp = true;

    if (it != _arp_cache.end()) {
        // send frame, no arp if has_reply
        if (it->second.has_reply) {
            send_arp = false;

            EthernetFrame dataframe;
            dataframe.header().src = _ethernet_address;
            dataframe.header().dst = it->second.addr;
            dataframe.header().type = TYPE_IPv4;
            dataframe.payload() = Buffer(dgram.serialize());

            _frames_out.push(dataframe);

            cout << "DEBUG: Found and has reply" << endl;
        }
        // no arp if retry timer hasn't expired
        else if (it->second.timer < ARP_RETRY_MS) {
            send_arp = false;
            cout << "DEBUG: Found, no reply, no retry" << endl;
        }
    }

    // either retry timer has expired or we don't have the address in cache
    if (send_arp) {
        // create/reset cache entry
        _arp_cache[next_hop_ip] = {false, 0, NULL};

        // send arp request
        _frames_out.push(_build_arpframe(next_hop_ip));
        
        // put datagram on waiting
        // TODO
    }


    // else create arp request and send, create cache entry, put ethernet frame on _frames_waiting
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    // check type of frame
    // if ipv4, exctract and return
    // if arp request, update cache, respond to request if it's us
    // if arp reply, update cache
    return {};
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    // iterate through all entries in _arp_cache and increment by tick
    // if we are above limit corresponding to has_reply (two diff limits) remove from the cache
}

// iterate through all waiting frames to see if we can send them now
void _flush_frames_waiting() {}

// update the cache by changing valid, resetting timer, or adding to cache
void _update_cache(uint32_t ipaddr, EthernetAddress ethaddr) {
    // flush waiting frames if we changed something to valid or added new entry
}

const EthernetFrame _build_arpframe(uint32_t ipaddr) {
    ARPMessage msg;
    msg.opcode = OPCODE_REQUEST;
    msg.sender_ethernet_address = _ethernet_address;
    msg.sender_ip_address = _ip_address.ipv4_numeric();
    msg.target_ip_address = ipaddr;

    EthernetFrame frame;
    frame.header().src = _ethernet_address;
    frame.header().dst = ETHERNET_BROADCAST;
    frame.header().type = TYPE_ARP;
    frame.payload() = Buffer(msg.serialize());

    return frame;
}



