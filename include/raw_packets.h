#ifndef RAW_PACKETS_DEF
#define RAW_PACKETS_DEF

#include <stdint.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <atomic>
#include <netinet/ip_icmp.h>

namespace raw_packets{

uint16_t calc_checksum(uint16_t *addr, int len);
bool send_syn(int sockfd, const sockaddr_in& src, sockaddr_in* dest,
    uint16_t src_port, uint16_t dest_port);
bool make_raw_socket(int& sd, int type);

int get_id();
void send_echo(int sockfd, int id, int seq, 
        sockaddr_in *dest_addr, int addr_len);
ssize_t recv_reply(int sockfd, msghdr *msg);
bool get_echo(char *ptr, ssize_t len, in_addr& src_ip);
bool send_ttl_1(int& resfd, int& id);
bool get_exceed_node(int fd, char* ptr, ssize_t len, int id, 
    in_addr& res_addr);

}

#endif // !RAW_PACKETS_DEF
