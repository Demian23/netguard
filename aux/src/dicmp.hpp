#ifndef DICMP_DEF
#define DICMP_DEF

#include "dnet.hpp"
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

namespace DICMP{
    bool make_icmp_socket(int &sockfd);
    void send_echo(int sockfd, int id, int seq, struct 
        sockaddr_in *dest_addr, int addr_len);
    ssize_t recv_reply(int sockfd, struct msghdr *msg);
    bool process_reply(int sockfd, char *ptr, ssize_t len, int id);
};

#endif
