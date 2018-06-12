#ifndef MONITOR_IP_HPP
#define MONITOR_IP_HPP

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

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

  std::string str() const {
    if (is_v4) {
      char buf[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &v4, buf, INET_ADDRSTRLEN);
      return std::string(buf, INET_ADDRSTRLEN);
    }
    else {
      char buf[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, &v6, buf, INET6_ADDRSTRLEN);
      return std::string(buf, INET6_ADDRSTRLEN);
    }
  }

  size_t hash() const {
    if (is_v4)
      return std::hash<uint32_t>()(v4.s_addr); //std::hash<int32_t>()(*(int32_t*)&v4);
    else {
      uint64_t x = 0;
      for (int i = 0; i < 4; ++i)
        x += v6.s6_addr[i];
      return std::hash<uint32_t>()(x);
    }
  }

  bool operator==(const IPAddr& x) const {
    if (is_v4 != x.is_v4)
      return false;
    if (is_v4)
      return v4.s_addr == x.v4.s_addr;
    else
      return v6.s6_addr == x.v6.s6_addr;
  }

  // struct hash {
  //   size_t operator()(const monitor::IPAddr& x) const {
  //     if (x.is_v4)
  //       return std::hash<int64_t>()(*(int64_t*)&x.v4);
  //     else
  //       return std::hash<int64_t>()(*(int64_t*)&x.v6);
  //   }
  // };
};

class PacketIP {
public:
  PacketIP(const u_char* packet, size_t pkt_len, size_t cap_len);

  bool is_tcp() const {return proto == TCP;}
  bool is_udp() const {return proto == UDP;}
  bool is_v4() const {return version == IPv4;}
  IPAddr get_src_addr() const {return src;}
  IPAddr get_dst_addr() const {return dst;}

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

// namespace std {
//   template <>
//   struct hash<monitor::IPAddr> {
//     size_t operator()(const monitor::IPAddr& x) const {
//       if (x.is_v4)
//         return hash<int64_t>()(*(int64_t*)&x.v4);
//       else
//         return hash<int64_t>()(*(int64_t*)&x.v6);
//     }
//   };
// } // namespace std

#endif // #ifndef MONITOR_IP_HPP
