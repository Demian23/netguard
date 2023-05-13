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
#include <fcntl.h>
#include <net/if.h>

namespace raw_packets{

uint16_t calc_checksum(uint16_t *addr, int len);
bool send_tcp_flag(int sockfd, const sockaddr_in& src, sockaddr_in* dest,
    uint16_t src_port, uint16_t dest_port, uint8_t flags);
bool get_syn_answer(char* packet, int len, sockaddr_in* from);
bool make_raw_socket(int& sd, int type);
bool bind_socket_to_interface(int sd, const char* interface);

int get_id();
void send_echo(int sockfd, int id, int seq, 
    sockaddr_in *dest_addr, int addr_len);
ssize_t recieve_packet(int sockfd, char* buffer, int buffer_size, 
    sockaddr_in* src);
bool get_echo(char *ptr, ssize_t len, in_addr& src_ip);
bool send_ttl_1(int& resfd, int& id);
bool get_exceed_node(int fd, char* ptr, ssize_t len, int id, 
    in_addr& res_addr);

}

#endif // !RAW_PACKETS_DEF