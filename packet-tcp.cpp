#include "packet-tcp.hpp"

#include <cassert>

#include <netinet/tcp.h>

namespace monitor {

PacketTCP::PacketTCP(const u_char* packet, size_t pkt_len, size_t cap_len)
  : packet(packet),
    pkt_len(pkt_len),
    cap_len(cap_len),
    src_port(0),
    dst_port(0),
    seq_num(0),
    ack_num(0),
    fl_fin(), fl_syn(), fl_rst(), fl_push(), fl_ack(), fl_urg(),
#ifdef __APPLE__
    fl_ece(0), fl_cwr(0),
#endif
    hdr_len(0)
{
  assert(pkt_len >= cap_len);
  assert(cap_len >= 20);
  tcphdr* tcp = (tcphdr*) packet;

  src_port = ntohs(tcp->th_sport);
  dst_port = ntohs(tcp->th_dport);

  seq_num = ntohl(tcp->th_seq);
  ack_num = ntohl(tcp->th_ack);

  // Read flags
  fl_fin = tcp->th_flags & TH_FIN;
  fl_syn = tcp->th_flags & TH_SYN;
  fl_rst = tcp->th_flags & TH_RST;
  fl_push = tcp->th_flags & TH_PUSH;
  fl_ack = tcp->th_flags & TH_ACK;
  fl_urg = tcp->th_flags & TH_URG;
#ifdef __APPLE__
  // For some reason linux does not define these flags
  fl_ece = tcp->th_flags & TH_ECE;
  fl_cwr = tcp->th_flags & TH_CWR;
#endif

  size_t offset = tcp->th_off;
  if (offset > 5) {
    // Has options
    assert(cap_len >= offset * 4);
    // We don't parse options yet
  }
}

} // namespace monitor
