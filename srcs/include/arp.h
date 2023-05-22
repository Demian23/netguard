#ifndef ARP_DEF
#define ARP_DEF

#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <unordered_map>

namespace ARP{
    typedef std::unordered_map<std::string, std::string> ip_mac_map;
    bool set_bpf_arp(int &fd, int &buflen, const char *interface);
    void writequery(int fd, ether_addr *ownmac, 
        sockaddr_in *ownip, const char *ip);
    bool collectresponse(int fd, ip_mac_map &map, char *buffer, int buflen);

};
#endif 
