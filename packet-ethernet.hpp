#ifndef MONITOR_ETHERNET_HPP
#define MONITOR_ETHERNET_HPP

#include <cstdint>
#include <net/ethernet.h>

#include "packet-ip.hpp"

namespace monitor {

class PacketEthernet {
public:
  PacketEthernet(const u_char* packet, size_t pkt_len, size_t cap_len);

  bool is_ip() const;
  bool is_arp() const;
  bool is_revarp() const;

  PacketIP get_ip() const;
private:
  const u_char* packet;
  size_t pkt_len, cap_len;
  uint8_t shost[ETHER_ADDR_LEN];
  uint8_t dhost[ETHER_ADDR_LEN];
  uint16_t type;
};

} // namespace monitor

#endif // #ifndef MONITOR_ETHERNET_HPP
