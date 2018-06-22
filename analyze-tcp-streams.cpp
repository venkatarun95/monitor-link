#include "analyze-tcp-streams.hpp"

#include "packet-tcp.hpp"
#include "tcp-connection.hpp"

#include <iostream>

using namespace std;

namespace monitor {

void AnalyzeTCPStreams::new_pkt(double timestamp, const PacketIP& ip) {
  if (!ip.is_tcp())
    return;
  // TODO(venkat): Handle fragmentation

  PacketTCP tcp = ip.get_tcp();

  FiveTuple five_tuple = {ip.get_src_addr(), ip.get_dst_addr(),
                          tcp.get_src_port(), tcp.get_dst_port()};
  FiveTuple five_tuple_rev =  {ip.get_dst_addr(), ip.get_src_addr(),
                               tcp.get_dst_port(), tcp.get_src_port()};

  if (tcp_conns.find(five_tuple) == tcp_conns.end())
    tcp_conns[five_tuple] = TCPConnection();
  if (tcp_conns.find(five_tuple_rev) == tcp_conns.end())
    tcp_conns[five_tuple_rev] = TCPConnection();

  tcp_conns[five_tuple].new_pkt(timestamp, tcp, false);
  tcp_conns[five_tuple_rev].new_pkt(timestamp, tcp, true);
}

void AnalyzeTCPStreams::print_conns(ostream& stream) const {
  stream << "Printing " << endl;
  for (const auto& x : tcp_conns) {
    stream << x.first.src.str() << ":" << x.first.sport << "-"
           << x.first.dst.str() << ":" << x.first.dport << " "
           << "AvgRTT "<< x.second.get_avg_rtt() << " "
           << "RTTVar " << x.second.get_rtt_var() << " "
           << "MedRTT " << x.second.get_median_rtt() << " "
           << "AvgTpt " << x.second.get_avg_tpt() << " "
           << "NumRtx " << x.second.get_num_rtx() << " "
           << "CntRTT " << x.second.get_num_rtt_samples() << " "
           << "NumPkt " << x.second.get_tot_num_pkts() << endl;
  }
}

} // namespace monitor
