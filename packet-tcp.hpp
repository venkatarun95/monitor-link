#ifndef MONITOR_TCP_HPP
#define MONITOR_TCP_HPP

#include <netinet/in.h>

namespace monitor {

class PacketTCP {
public:
  PacketTCP(const u_char* packet, size_t pkt_len, size_t cap_len);

  uint16_t get_src_port() const {return src_port;}
  uint16_t get_dst_port() const {return dst_port;}

  // Flag getters
  bool is_fin() const {return fl_fin;}
  bool is_syn() const {return fl_syn;}
  bool is_rst() const {return fl_rst;}
  bool is_push() const {return fl_push;}
  bool is_ack() const {return fl_ack;}
  bool is_urg() const {return fl_urg;}
#ifdef __APPLE__
  bool is_eve() const {return fl_ece;}
  bool is_cwr() const {return fl_cwr;}
#endif

  // Payload size in bytes
  uint32_t get_payload_size() const {return pkt_len - hdr_len;}
  uint32_t get_seq_num() const {return seq_num;}
  uint32_t get_ack_num() const {return ack_num;}

private:
  const u_char* packet;
  size_t pkt_len, cap_len;

  uint16_t src_port, dst_port;
  uint32_t seq_num, ack_num;
  // Flags
  bool fl_fin, fl_syn, fl_rst, fl_push, fl_ack, fl_urg;
#ifdef __APPLE__
  bool fl_ece, fl_cwr;
#endif
  size_t hdr_len;
};

} // namespace monitor

#endif // #ifndef MONITOR_TCP_HPP
