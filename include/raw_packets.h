#ifndef RAW_PACKETS_DEF
#define RAW_PACKETS_DEF

#include <stdint.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

namespace raw_packets{

uint16_t calc_checksum(uint16_t *addr, int len);
bool send_syn(int sockfd, const sockaddr_in& src, sockaddr_in* dest,
    uint16_t src_port, uint16_t dest_port);
bool make_raw_socket(int& sd, int type);

}

#endif // !RAW_PACKETS_DEF
