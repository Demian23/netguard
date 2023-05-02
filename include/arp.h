#ifndef ARP_DEF
#define ARP_DEF

#include <netinet/in.h>
#include <net/ethernet.h>
namespace ARP{
    struct arp_pair{
        ether_addr mac;
        sockaddr_in ip;
        arp_pair(const ether_addr& a_mac, const sockaddr_in& a_ip) 
            : mac(a_mac), ip(a_ip){}
        arp_pair(){}
    };
    bool set_bpf_arp(int &fd, int &buflen, const char *interface);
    void writequery(int fd, ether_addr *ownmac, 
        sockaddr_in *ownip, const char *ip);
    bool collectresponse(int fd, arp_pair &p, char *buffer, int buflen);

};
#endif 
