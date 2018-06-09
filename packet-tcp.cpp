#include "packet-tcp.hpp"

#include <cassert>

#include <netinet/tcp.h>

namespace monitor {

PacketTCP::PacketTCP(const u_char* packet, size_t pkt_len, size_t cap_len)
  : packet(packet),
    pkt_len(pkt_len),
    cap_len(cap_len)
{
  assert(pkt_len >= cap_len);
  assert(cap_len >= 20);
  tcphdr* tcp = (tcphdr*) packet;

  src_port = ntohs(tcp->th_sport);
  dst_port = ntohs(tcp->th_dport);

  seq_num = ntohs(tcp->th_seq);
  ack_num = ntohs(tcp->th_ack);

  // Read flags
  fl_fin = tcp->th_flags & TH_FIN;
  fl_syn = tcp->th_flags & TH_SYN;
  fl_rst = tcp->th_flags & TH_RST;
  fl_push = tcp->th_flags & TH_PUSH;
  fl_ack = tcp->th_flags & TH_ACK;
  fl_urg = tcp->th_flags & TH_URG;
  fl_ece = tcp->th_flags & TH_ECE;
  fl_cwr = tcp->th_flags & TH_CWR;

  size_t offset = tcp->th_off;
  if (offset > 5) {
    // Has options
    assert(cap_len >= offset * 4);
    // We don't parse options yet
  }
}

} // namespace monitor
