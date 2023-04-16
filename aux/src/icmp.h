#ifndef ICMP_DEF
#define ICMP_DEF

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

namespace ICMP{
    bool make_icmp_socket(int &sockfd);
    void send_echo(int sockfd, int id, int seq, 
        sockaddr_in *dest_addr, int addr_len);
    ssize_t recv_reply(int sockfd, msghdr *msg);
    bool process_reply(int sockfd, char *ptr, ssize_t len, int id);
};

#endif
