#ifndef MONITOR_TCP_CONNECTION_HPP
#define MONITOR_TCP_CONNECTION_HPP

#include "packet-tcp.hpp"

#include <deque>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/framework/features.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/weighted_mean.hpp>

namespace monitor {

class TCPConnection {
  typedef uint32_t Seq;
public:
  TCPConnection();
  // Register a new packet. 'ack' indicates whether we should look at this as an
  // ack or a sequence number
  void new_pkt(double timestamp, const PacketTCP& pkt, bool ack);

  // Units: data - bytes, time - sec
  double get_avg_rtt() const {return boost::accumulators::mean(rtt);}
  double get_rtt_var() const {return boost::accumulators::variance(rtt);}
  double get_median_rtt() const {return boost::accumulators::median(rtt);}
  double get_avg_tpt() const;

private:
  // Check if two ranges overlap (accounting for wraparound)
  bool overlaps(Seq start1, Seq len1, Seq start2, Seq len2) const;

  struct MetaData {
    // The start and end of this block of bytes (usually one packet)
    Seq start, len;
    double timestamp;
    enum {InFlight, Rtxd} status;
  };
  // Stores metadata about sequence numbers. The sequence numbers are
  // monotonically increasing (not counting wraparound) and never duplicated.
  std::deque<MetaData> pkts;

  boost::accumulators::accumulator_set<double,
                                       boost::accumulators::features<boost::accumulators::tag::mean,
                                                                     boost::accumulators::tag::variance,
                                                                     boost::accumulators::tag::median> > rtt;
  boost::accumulators::accumulator_set<double,
                                       boost::accumulators::features<boost::accumulators::tag::weighted_mean>, double> avg_throughput;
  // For calculating the throughput in the last transmit interval. A transmit
  // interval is a contiguous period of time when there were always packets in
  // flight
  double time_interval_start;
  int64_t num_bytes_in_interval;

  // Last time we got an ACK
  double last_ack_time;
};

} // namespace monitor

#endif // #ifndef MONITOR_TCP_CONNECTION_HPP
