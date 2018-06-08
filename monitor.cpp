#include <iostream>
#include <pcap.h>

#include <netinet/if_ether.h>

using namespace std;

// Callback function
void packet_handler(u_char *args, const struct pcap_pkthdr *header,
                const u_char *packet);

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

  // Start pcap loop
  pcap_loop(handle, -1, packet_handler, nullptr);
  
  return 0;
}

void packet_handler(u_char *args, const struct pcap_pkthdr *header,
                    const u_char *packet) {
  // Timestamp   in microseconds
  uint64_t ts = (uint64_t)header->ts.tv_sec * 1000000 +
    (uint64_t)header->ts.tv_usec;
  cout << "Timestamp " << ts * 1e-6 << " Length " << header->len << " Captured " << header->caplen << endl;

  // Parse ethernet header
  ether_header *eth = (ether_header*) packet;
  if (ntohs(eth->ether_type) == ETHERTYPE_IP) {
    cout << "IP packet" << endl;
  }
  else if (ntohs(eth->ether_type) == ETHERTYPE_ARP) {
    cout << "ARP packet" << endl;
  }
  else {
    cout << "Unrecognized packet type" << endl;
  }

  args = nullptr;
}
