#include "packet-ethernet.hpp"
#include "packet-ip.hpp"
#include "packet-tcp.hpp"
#include "tcp-connection.hpp"

#include <cstdlib>
#include <iostream>
#include <pcap.h>
#include <unordered_map>

#include <netinet/ip.h>

using namespace std;

// Callback function
void packet_handler(u_char *args, const struct pcap_pkthdr *header,
                const u_char *packet);

void print_tcp_conns();
void sig_ints_handler(int){print_tcp_conns();}

int main(int argc, char *argv[])
{
  if (argc != 2 && argc != 3) {
    cerr << "Usage: ./monitor <device> [<filter expression>]" << endl;
    return -1;
  }

  // Start capture
  char *dev_name = argv[1];
  cout << "Capturing on " << dev_name << endl;
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t *handle = pcap_open_live(dev_name, BUFSIZ, true, 1000, errbuf);
  if (handle == nullptr) {
    cerr << "Couldn't start capture on device " << dev_name << endl;
    return -1;
  }

  if (pcap_datalink(handle) != DLT_EN10MB) {
    cerr << "Device doesn't provide Ethernet headers. This is not supported" << endl;
    return -1;
  }

  // Find IP address and netmask
  bpf_u_int32 dev_netmask, dev_ip;
  if (pcap_lookupnet(dev_name, &dev_ip, &dev_netmask, errbuf) == -1) {
    cerr << "Couldn't find netmask and IP address for device "
         << dev_name <<  endl;
    return -1;
  }

  // Install filter
  if (argc == 3) {
    bpf_program filter;
    if (pcap_compile(handle, &filter, argv[3], 1, dev_netmask) == -1) {
      cerr << "Couldn't compile filter expression '" << argv[3] << "'." << endl;
      return -1;
    }
    if (pcap_setfilter(handle, &filter) == -1) {
      cerr << "Couldn't install filter expression." << endl;
      return -1;
    }
  }

  // SIGINT handler
  signal(SIGINT, &sig_ints_handler);

  // Start pcap loop
  pcap_loop(handle, -1, packet_handler, nullptr);
  
  return 0;
}

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

unordered_map<FiveTuple, monitor::TCPConnection, FiveTuple::hash> tcp_conns;

void packet_handler(u_char *args, const struct pcap_pkthdr *header,
                    const u_char *packet) {
  // Timestamp   in microseconds
  uint64_t ts = (uint64_t)header->ts.tv_sec * 1000000 +
    (uint64_t)header->ts.tv_usec;
  double timestamp = ts * 1e-6;

  monitor::PacketEthernet eth(packet, header->len, header->caplen);
  if (!eth.is_ip())
    return;

  monitor::PacketIP ip = eth.get_ip();
  if (!ip.is_tcp())
    return;
  // TODO(venkat): Handle fragmentation

  monitor::PacketTCP tcp = ip.get_tcp();

  FiveTuple five_tuple = {ip.get_src_addr(), ip.get_dst_addr(),
                          tcp.get_src_port(), tcp.get_dst_port()};
  FiveTuple five_tuple_rev =  {ip.get_dst_addr(), ip.get_src_addr(),
                               tcp.get_dst_port(), tcp.get_src_port()};

  if (tcp_conns.find(five_tuple) == tcp_conns.end())
    tcp_conns[five_tuple] = monitor::TCPConnection();
  if (tcp_conns.find(five_tuple_rev) == tcp_conns.end())
    tcp_conns[five_tuple_rev] = monitor::TCPConnection();

  tcp_conns[five_tuple].new_pkt(timestamp, tcp, false);
  tcp_conns[five_tuple_rev].new_pkt(timestamp, tcp, true);
  args = nullptr;
}

void print_tcp_conns() {
  cout << "Printing " << endl;
  for (const auto& x : tcp_conns) {
    cout << x.first.src.str() << ":" << x.first.sport << "-"
         << x.first.dst.str() << ":" << x.first.dport << " "
         << x.second.get_avg_rtt() << " "
         << x.second.get_rtt_var() << " "
         << x.second.get_median_rtt() << " "
         << x.second.get_avg_tpt() << endl;
      }
  exit(0);
}
