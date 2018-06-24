#ifndef MONITOR_ANALYZE_TCP_STREAMS_HH
#define MONITOR_ANALYZE_TCP_STREAMS_HH

#include "packet-ip.hpp"
#include "tcp-connection.hpp"

#include <fstream>
#include <unordered_map>

namespace monitor {

class AnalyzeTCPStreams {
public:
  AnalyzeTCPStreams() :
    tcp_conns(),
    tot_pkts(0),
    num_non_tcp_pkts(0),
    num_frag_pkts(0),
    num_small_pkts(0)
  {}
  // Timestamp in microseconds
  void new_pkt(uint64_t timestamp, const PacketIP& ip);
  void print_conns(std::ostream&) const;

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

  uint64_t start_time;
  uint64_t tot_pkts;
  uint64_t num_non_tcp_pkts;
  uint64_t num_frag_pkts;
  uint64_t num_small_pkts;
};

} // namespace monitor

#endif
