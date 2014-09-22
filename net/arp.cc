/*
 * Copyright (C) 2014 Cloudius Systems, Ltd.
 */

#include "arp.hh"

namespace net {

arp_for_protocol::arp_for_protocol(arp& a, uint16_t proto_num)
    : _arp(a), _proto_num(proto_num) {
    _arp.add(proto_num, this);
}

arp_for_protocol::~arp_for_protocol() {
    _arp.del(_proto_num);
}

arp::arp(interface* netif) : _netif(netif), _proto(netif, 0x0806)
    , _rx_packets(_proto.receive([this] (packet p, ethernet_address ea) {
        return process_packet(std::move(p), ea);
    })) {
}

void arp::add(uint16_t proto_num, arp_for_protocol* afp) {
    _arp_for_protocol[proto_num] = afp;
}

void arp::del(uint16_t proto_num) {
    _arp_for_protocol.erase(proto_num);
}

future<>
arp::process_packet(packet p, ethernet_address from) {
    auto ah = p.get_header<arp_hdr>();
    ntoh(*ah);
    auto i = _arp_for_protocol.find(ah->ptype);
    hton(*ah); // return to raw state for further processing
    if (i != _arp_for_protocol.end()) {
        i->second->received(std::move(p));
    }
    return make_ready_future<>();
}

}
