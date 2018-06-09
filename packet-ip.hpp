#ifndef MONITOR_IP_HPP
#define MONITOR_IP_HPP

#include <netinet/in.h>

#include "packet-tcp.hpp"

namespace monitor {

struct IPAddr {
  union {
    in_addr  v4;
    in6_addr v6;
  };
  bool is_v4;

  IPAddr() :
    v4(),
    is_v4(true)
  {}
  IPAddr(in_addr a) :
    v4(a),
    is_v4(true)
  {}
  IPAddr(in6_addr a) :
    v6(a),
    is_v4(false)
  {}
};

class PacketIP {
public:
  PacketIP(const u_char* packet, size_t pkt_len, size_t cap_len);

  bool is_tcp() const {return proto == TCP;}
  bool is_udp() const {return proto == UDP;}
  IPAddr src_addr() const {return src;}
  IPAddr dst_addr() const {return dst;}
  PacketTCP get_tcp() const;

private:
  const u_char* packet;
  size_t pkt_len, cap_len;
  enum IP_VERSION {IPv4, IPv6} version;
  enum PROTO {TCP, UDP, Unknown} proto;
  IPAddr src, dst;
  size_t hdr_len;
};

} // namespace monitor

#endif // #ifndef MONITOR_IP_HPP
