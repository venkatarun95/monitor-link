#include "ip.hpp"

#include <cassert>
#include <iostream>

#include <netinet/ip.h>

namespace monitor {

IP::IP(const u_char* packet, size_t pkt_len, size_t cap_len) {
#ifdef _IP_VHL
  // Version and leader length fields combined. Not yet supported.
  assert(false);
#endif
  assert(pkt_len >= cap_len);
  // Get the version and header length first
  assert(cap_len > 1);
  ip* ip_hdr = (ip*) packet;

  // Handle IP versions
  if (ip_hdr->ip_v == 4)
    version = IPv4;
  else if (ip_hdr->ip_v == 6)
    version = IPv6;
  else
    assert(false); // Unrecognized IP version

  if (version == IPv6) {
    std::cerr << "IPV6 not yet supported" << std::endl;
    exit(1);
  }

  // Check that our capture length is bigger than address
  size_t hdr_len = ip_hdr->ip_hl;
  assert(cap_len > hdr_len);

  // Get src and dst address
  src = ip_hdr->ip_src;
  dst = ip_hdr->ip_dst;

  // Type of transport layer
  switch(ip_hdr->ip_p) {
  case 0x06: proto = TCP; break;
  case 0x11: proto = UDP; break;
  default: proto = Unknown; break;
  }
}

} // namespace monitor
