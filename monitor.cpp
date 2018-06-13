#include "analyze-tcp-streams.hpp"
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

monitor::AnalyzeTCPStreams analyze_tcp;
int link_type = -1;

// Callback function
void packet_handler(u_char *args, const struct pcap_pkthdr *header,
                const u_char *packet);

void print_tcp_conns();
void sig_ints_handler(int){analyze_tcp.print_conns(); exit(0);}

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

  // Determine the type of link-layer header
  link_type = pcap_datalink(handle);
  switch (link_type) {
  case DLT_NULL:
    cout << "Link-layer type: NULL e.g. used in BSD loopback" << endl;
    break;
  case DLT_RAW:
    cout << "Link-layer type: Raw IP headers" << endl;
    break;
  case DLT_EN10MB:
    cout << "Link-layer type: Ethernet" << endl;
    break;
  case DLT_LINUX_SLL:
    cout << "Link-layer type: Linux \"cooked\" capture encapsulation" << endl;
    cerr << "Error: Link-layer type not yet supported." << endl;
    return -1;
    break;
  case DLT_IEEE802_11_RADIO:
    cout << "Link-layer type: RadioTap header" << endl;
    cerr << "Error: Link-layer type not yet supported." << endl;
    return -1;
    break;
  default:
    cerr << "Unsupported link-layer type '" << link_type << "'." << endl;
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

struct LinkLayer_LinuxSLL {
  // Who sent packet to whom (us/somebody else/broadcast/multicast)
  uint16_t packet_type;
  uint16_t arphrd_type;
  // ... header incomplete ...
};

void packet_handler(u_char *args, const struct pcap_pkthdr *header,
                    const u_char *packet) {
  // Timestamp   in microseconds
  uint64_t ts = (uint64_t)header->ts.tv_sec * 1000000 +
    (uint64_t)header->ts.tv_usec;
  double timestamp = ts * 1e-6;

  // Decode link layer to get IP packet
  monitor::PacketIP ip;
  switch (link_type) {
  case DLT_EN10MB: {
    monitor::PacketEthernet eth(packet, header->len, header->caplen);
    if (!eth.is_ip())
      return;
    ip = eth.get_ip();
    break;
  }
  case DLT_RAW:
    ip = monitor::PacketIP(packet, header->len, header->caplen);
    break;
  case DLT_NULL: {
    uint32_t pkt_type = *(uint32_t*) packet;
    if (pkt_type == 2 || pkt_type == 24 || pkt_type == 28 || pkt_type == 30)
      ip = monitor::PacketIP(packet + 4, header->len - 4, header->caplen - 4);
    break;
  }
  case DLT_IEEE802_11_RADIO: {
    
  }
  default:
    assert(false); // Link type not handled
  }

  analyze_tcp.new_pkt(timestamp, ip);

  args = nullptr; // To prevent 'attribute unused' warning
}


