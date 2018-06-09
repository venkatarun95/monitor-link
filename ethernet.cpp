#include "ethernet.hpp"

#include <cassert>
#include <cstring>

#include <netinet/if_ether.h>

namespace monitor {

Ethernet::Ethernet(const u_char* packet, size_t pkt_len, size_t cap_len)
  : packet(packet),
    pkt_len(pkt_len),
    cap_len(cap_len)
{
  assert(pkt_len >= cap_len);
  assert(cap_len >= sizeof(ether_header));
  ether_header *eth = (ether_header*) packet;
  memcpy(shost, eth->ether_shost, ETHER_ADDR_LEN);
  memcpy(dhost, eth->ether_dhost, ETHER_ADDR_LEN);
  type = eth->ether_type;
}

IP Ethernet::get_ip() const {
  size_t len = sizeof(ether_header);
  return IP(packet+len, pkt_len-len, cap_len-len);
}

bool Ethernet::is_ip() const {
  return ntohs(type) == ETHERTYPE_IP;
}

bool Ethernet::is_arp() const {
  return ntohs(type) == ETHERTYPE_ARP;
}

bool Ethernet::is_revarp() const {
  return ntohs(type) == ETHERTYPE_REVARP;
}

} // namespace monitor
