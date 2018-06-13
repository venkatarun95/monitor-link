#ifndef MONITOR_ANALYZE_TCP_STREAMS_HH
#define MONITOR_ANALYZE_TCP_STREAMS_HH

#include "packet-ip.hpp"
#include "tcp-connection.hpp"

#include <unordered_map>

namespace monitor {

class AnalyzeTCPStreams {
public:
  void new_pkt(double timestamp, const PacketIP& ip);
  void print_conns() const;

private:
  struct FiveTuple {
    monitor::IPAddr src, dst;
    uint16_t sport, dport;
    // Always TCP, no need to store protocol

    struct hash {
      size_t operator()(const FiveTuple& x) const {
        return x.src.hash() ^ x.dst.hash() ^
          std::hash<uint32_t>()(x.sport * x.dport);
      }
    };

    bool operator==(const FiveTuple& x) const {
      return src == x.src &&
        dst == x.dst &&
        sport == x.sport && dport == x.dport;
    }
  };

  std::unordered_map<FiveTuple, monitor::TCPConnection, FiveTuple::hash> tcp_conns;
};

} // namespace monitor

#endif
