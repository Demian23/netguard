#ifndef DARP_DEF
#define DARP_DEF

#include "dnet.hpp"

namespace DARP{
    enum{arp_timeout = 500000};
    struct arp_pair{
        struct ether_addr mac;
        struct sockaddr_in ip;
    };
    void set_timeout(int fd);
    void set_nonblock(int fd);
    bool set_bpf_arp(int &fd, int &buflen, const char *interface);
    void writequery(int fd, struct ether_addr *ownmac, 
        struct sockaddr_in *ownip, char *ip);
    bool collectresponse(int fd, arp_pair &p, char *buffer, int buflen);

};
#endif 
