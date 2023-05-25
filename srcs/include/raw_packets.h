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
#include <netinet/ip_icmp.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/bpf.h>
#include <netinet/if_ether.h>
#include <string.h>

namespace raw_packets{

uint16_t calc_checksum(uint16_t *addr, int len);
bool make_raw_socket(int& sd, int type);
bool make_manual_socket(int& sd, int type);

bool send_tcp_flag(int sockfd, sockaddr_in* src, sockaddr_in* dest,
    uint8_t flags, char* packet_buffer);

bool send_irc(int& resfd, int& id);

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
