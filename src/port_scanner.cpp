#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
/*
#include "../include/port_scanner.h"
static int id;
void setup_datagram(char* datagram, int datagram_size, 
    in_addr self, in_addr dest, int dest_port)
{
    ip ip_hdr;
    ip_hdr.ip_p = IPPROTO_TCP;
    ip_hdr.ip_v = 4;
    ip_hdr.ip_hl = 5;
    ip_hdr.ip_id = ++id;
    ip_hdr.ip_dst = dest;
    ip_hdr.ip_src = self;
    ip_hdr.ip_len = sizeof(ip) + sizeof(tcphdr);
    //ip_hdr.ip_sum = ICMP::calc_checksum(uint16_t *addr, int len) 

    tcphdr tcp;
    tcp.th_sport = htons(46300);
    tcp.th_dport = htons(dest_port);
    tcp.th_win = htons(14600);
    
}
*/
