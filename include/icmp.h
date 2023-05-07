#ifndef ICMP_DEF
#define ICMP_DEF

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

namespace ICMP{
    enum Errors{Allright, NotEcho, WrongId, NotIcmp};
    int get_id();
    bool make_icmp_socket(int &sockfd);
    void send_echo(int sockfd, int id, int seq, 
            sockaddr_in *dest_addr, int addr_len);
    ssize_t recv_reply(int sockfd, msghdr *msg);
    Errors get_echo(char *ptr, ssize_t len, in_addr& src_ip);
    bool send_ttl_1(int& resfd, int& id);
    bool get_exceed_node(int fd, char* ptr, ssize_t len, int id, 
        in_addr& res_addr);
    uint16_t calc_checksum(uint16_t *addr, int len);

};

#endif
