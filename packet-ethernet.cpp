#include "packet-ethernet.hpp"

#include <cassert>
#include <cstring>

#include <netinet/if_ether.h>

namespace monitor {

PacketEthernet::PacketEthernet(const u_char* packet, size_t pkt_len,
                               size_t cap_len)
  : packet(packet),
    pkt_len(pkt_len),
    cap_len(cap_len),
    shost(),
    dhost(),
    type()
{
  assert(pkt_len >= cap_len);
  assert(cap_len >= sizeof(ether_header));
  ether_header *eth = (ether_header*) packet;
  memcpy(shost, eth->ether_shost, ETHER_ADDR_LEN);
  memcpy(dhost, eth->ether_dhost, ETHER_ADDR_LEN);
  type = eth->ether_type;
}

PacketIP PacketEthernet::get_ip() const {
  assert(is_ip());
  size_t len = sizeof(ether_header);
  return PacketIP(packet+len, pkt_len-len, cap_len-len);
}

bool PacketEthernet::is_ip() const {
  return ntohs(type) == ETHERTYPE_IP || ntohs(type) == ETHERTYPE_IPV6;
}

bool PacketEthernet::is_arp() const {
  return ntohs(type) == ETHERTYPE_ARP;
}

bool PacketEthernet::is_revarp() const {
  return ntohs(type) == ETHERTYPE_REVARP;
}

} // namespace monitor
