#include "tcp-connection.hpp"

#include <iostream>
#include <limits>

using namespace std;

namespace monitor {

TCPConnection::TCPConnection() :
  pkts(),
  rtt(),
  avg_throughput(),
  time_interval_start(0.),
  interval_start_ack_num(0),
  last_acked_pkt(0),
  last_ack_time(0.),
  tot_num_pkts(0),
  num_rtx(0)
{}

bool TCPConnection::overlaps(Seq start1, Seq len1, Seq start2, Seq len2) const {
  // No wraparound
  if (start1 <= start1 + len1 && start2 <= start2 + len2) {
    if (start1 <= start2 && start2 <= start1 + len1)
      return true;
    return false;
  }
  else {
    start1 -= numeric_limits<Seq>::max() / 2;
    start2 -= numeric_limits<Seq>::max() / 2;
    return overlaps(start1, len1, start2, len2);
  }
}

void TCPConnection::new_pkt(double timestamp, const PacketTCP& pkt, bool ack) {
  ++ tot_num_pkts;
  if (!ack) {
    bool is_retransmission = false;
    // Check if it is a retransmission
    if (!pkts.empty() &&
        pkt.get_seq_num() < pkts.back().start + pkts.back().len) {
      // Mark that packet as a retransmission
      for (auto& x : pkts) {
        if (overlaps(x.start, x.len, pkt.get_seq_num(), pkt.get_payload_size())) {
          x.status = MetaData::Rtxd;
          is_retransmission = true;
        }
        if (x.start + x.len < pkt.get_seq_num())
          break;
      }
    }
    if (!is_retransmission) {
      pkts.push_back(MetaData {
          pkt.get_seq_num(),
          pkt.get_payload_size(),
          timestamp,
          MetaData::InFlight
          });
    }
    else
      ++ num_rtx;
  }
  else {
    // Pop acked packets and compute RTT
    while (!pkts.empty()) {
      if (pkts.front().start + pkts.front().len == pkt.get_ack_num())
        if (pkts.front().status == MetaData::InFlight)
          rtt(timestamp - pkts.front().timestamp);
      if (pkts.front().start + pkts.front().len <= pkt.get_ack_num())
        pkts.pop_front();
      else
        break;
    }

    // TODO(venkat): handle wraparound
    double prev_last_acked_pkt = last_acked_pkt;
    // Updated last acked packet, taking wraparound into account
    if (pkt.get_ack_num() > last_acked_pkt ||
        (last_acked_pkt >= numeric_limits<Seq>::max()/2 &&
         last_acked_pkt - pkt.get_ack_num() >= numeric_limits<Seq>::max()/2))
      last_acked_pkt = pkt.get_ack_num();

    const double timeout = 2.0;
    // No time interval currently active
    if (time_interval_start == 0.) {
      // Activate interval
      time_interval_start = timestamp;
      interval_start_ack_num = last_acked_pkt;
      last_ack_time = timestamp;
      return;
    }
    // See if last time interval ended
    else if (timestamp - last_ack_time > timeout ||
             last_acked_pkt - interval_start_ack_num >= numeric_limits<Seq>::max()/2) {
      // This is the first packet in next interval
      double interval = last_ack_time - time_interval_start;
      Seq num_bytes_in_interval = prev_last_acked_pkt - interval_start_ack_num;
      if (num_bytes_in_interval > 0 && interval > 0)
        avg_throughput(num_bytes_in_interval / interval,
                       boost::accumulators::weight=interval);
      if (pkt.get_ack_num() > last_acked_pkt) {
        time_interval_start = timestamp;
        interval_start_ack_num = last_acked_pkt;
      }
      else
        time_interval_start = 0;
    }

    last_ack_time = timestamp;
  }
}

double TCPConnection::get_avg_tpt() const {
  using namespace boost::accumulators;
  Seq num_bytes_in_interval = last_acked_pkt - interval_start_ack_num;
  return 8 * (weighted_sum(avg_throughput) + num_bytes_in_interval) /
    (sum_of_weights(avg_throughput) + last_ack_time - time_interval_start);
}

}
