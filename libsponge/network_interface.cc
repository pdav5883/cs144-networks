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
        // address is in cache with reply -- send ethernet
        if (it->second.has_reply) {
            send_arp = false;

            EthernetFrame dataframe;
            dataframe.header().src = _ethernet_address;
            dataframe.header().dst = it->second.addr;
            dataframe.header().type = EthernetHeader::TYPE_IPv4;
            dataframe.payload() = Buffer(dgram.serialize());

            _frames_out.push(dataframe);

            cout << "DEBUG: Found and has reply" << endl;
        }
        // address is in cache, timer not expired -- keep waiting
        else if (it->second.timer < ARP_RETRY_MS) {
            send_arp = false;
            cout << "DEBUG: Found, no reply, no retry" << endl;
        }
    }

    // either retry timer has expired or we don't have the address in cache
    if (send_arp) {
        // create/reset cache entry
        cache_value val = {false, 0, NULL}; // TODO may not need
        _arp_cache[next_hop_ip] = val;

        // send arp request
        _frames_out.push(_build_arprequest_frame(next_hop_ip));
        
        // put datagram on waiting
        wait_item item = {dgram, next_hop_ip}; // TODO may not need
        _waiting.push_back(item);
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    // frame must have relevant destination
    EthernetAddress dst = frame.header().dst
    if (dst != _ethernet_address && dst != ETHERNET_BROADCAST) {
        return {};
    }

    // if ipv4, extract and return
    if (frame.header().type == EthernetHeader::TYPE_IPv4) {
        IPv4Datagram dgram;
        if (dgram.parse(frame.payload()) != ParseResult::NoError) {
            return {};
        }
        
        return dgram;
    }

    // if arp message, depends on request/reply
    else if (frame.header().type == EthernetHeader::TYPE_ARP) {
        ARPMessage msg;
        if (msg.parse(frame.payload()) != ParseResult::NoError) {
            return {};
        }

        // always record sender mapping in cache
        _update_cache(msg.sender_ip_address, msg.sender_ethernet_address);

        // send reply if msg is request and we are target
        if (msg.opcode == ARPMessage::OPCODE_REQUEST && msg.target_ip_address == _ip_address.ipv4_numeric()) {
            _frames_out.push(_build_arpreply_frame(msg.sender_ip_address, msg.sender_ethernet_address));
        }

        return {};
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    // iterate through all entries in _arp_cache and increment by tick
    // if we are above limit corresponding to has_reply (two diff limits) remove from the cache
}

// iterate through all waiting frames to see if we can send them now
void _flush_waiting() {}

// update the cache by changing valid, resetting timer, or adding to cache
void _update_cache(const uint32_t ipaddr, const EthernetAddress &ethernet_address) {
    // flush waiting frames if we changed something to valid or added new entry
}

const EthernetFrame _build_arprequest_frame(const uint32_t ipaddr) {
    // ip addr that we are looking for matching eth addr
    ARPMessage msg;
    msg.opcode = ARPMessage::OPCODE_REQUEST;
    msg.sender_ethernet_address = _ethernet_address;
    msg.sender_ip_address = _ip_address.ipv4_numeric();
    msg.target_ip_address = ipaddr;

    EthernetFrame frame;
    frame.header().src = _ethernet_address;
    frame.header().dst = ETHERNET_BROADCAST;
    frame.header().type = EthernetHeader::TYPE_ARP;
    frame.payload() = Buffer(msg.serialize());

    return frame;
}

const EthernetFrame _build_arpreply_frame(const uint32_t ipaddr, const EthernetAddress &ethernet_address) {
    // ip/ether addr of the node we are replying to
    ARPMessage msg;
    msg.opcode = ARPMessage::OPCODE_REPLY;
    msg.sender_ethernet_address = _ethernet_address;
    msg.sender_ip_address = _ip_address.ipv4_numeric();
    msg.target_ethernet_address = ethernet_address;
    msg.target_ip_address = ipaddr;

    EthernetFrame frame;
    frame.header().src = _ethernet_address;
    frame.header().dst = ethernet_address;
    frame.header().type = EthernetHeader::TYPE_ARP;
    frame.payload() = Buffer(msg.serialize());

    return frame;
}



