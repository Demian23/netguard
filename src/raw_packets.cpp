#include "../include/raw_packets.h"
#include "../include/errors.h"
#include <netinet/in.h>

namespace raw_packets{

enum{default_send_port = 48888, default_buffer_size = 1024};

struct pseudo_header    //needed for checksum calculation in syn
{
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t tcp_length;
     
    tcphdr tcp;
};

bool make_raw_socket(int& sd, int type)
{
   bool res = true;
   if(-1 == (sd = socket(AF_INET, SOCK_RAW, type))){
       errors::SysRet("Fail to create raw socket");
       res = false;
   }
   return res;
}

uint16_t calc_checksum(uint16_t *addr, int len)
{
   int nleft = len; 
   uint32_t sum = 0;
   uint16_t *w = addr;
   uint16_t answer = 0;

   while(nleft > 1){
       sum += *w++;
       nleft -= 2;
   }
   if(nleft == 1){
        *(unsigned char*)(&answer) = *(unsigned char *)w; 
        sum += answer;
   }
   sum = (sum >> 16) + (sum & 0xffff);
   sum += (sum >> 16);
   answer = ~sum;
   return answer;
}

//return result packet size, assumed that packet buffer is big enough
int prepare_syn_packet(char* packet, in_addr src, in_addr dest, 
    uint16_t src_port, uint16_t dest_port)
{
    ip *ip_hdr = reinterpret_cast<ip*>(packet);
    tcphdr* tcp_hdr = reinterpret_cast<tcphdr*>(packet + sizeof(ip));
    pseudo_header psh;

    ip_hdr->ip_hl = 5;
    ip_hdr->ip_v = IPVERSION;
    ip_hdr->ip_tos = 0;
    ip_hdr->ip_len = sizeof(ip) + sizeof(tcphdr);
    ip_hdr->ip_id = 0;
    ip_hdr->ip_off = 0;
    ip_hdr->ip_ttl = 64;
    ip_hdr->ip_p = IPPROTO_TCP;
    ip_hdr->ip_sum = 0;
    ip_hdr->ip_src = src;
    ip_hdr->ip_dst = dest;
    ip_hdr->ip_sum = calc_checksum(reinterpret_cast<uint16_t*>(packet), 
        ip_hdr->ip_len >> 1);

    tcp_hdr->th_sport = htons(src_port); 
    tcp_hdr->th_dport = htons(dest_port);
    tcp_hdr->th_seq = 0;
    tcp_hdr->th_ack = 0;
    tcp_hdr->th_off = 5;
    tcp_hdr->th_flags = TH_SYN;
    tcp_hdr->th_win = htons(1460);
    tcp_hdr->th_sum = 0;
    tcp_hdr->th_urp = 0;

    psh.source_address = src.s_addr;
    psh.dest_address = dest.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(20);

    memcpy(&psh.tcp, tcp_hdr, sizeof(tcphdr));
    tcp_hdr->th_sum = calc_checksum(reinterpret_cast<uint16_t*>(&psh), 
        sizeof(pseudo_header));
    return ip_hdr->ip_len;
}

bool send_syn(int sockfd, const sockaddr_in& src, sockaddr_in* dest,
    uint16_t src_port, uint16_t dest_port)
{
    char packet[default_buffer_size] = {}; 
    bool res = true;
    int packet_len = prepare_syn_packet(packet, src.sin_addr, 
        dest->sin_addr, default_send_port, dest_port);
    int opt = 1;
    ssize_t ret = setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &opt, 
        sizeof(int));
    if(ret != -1)
        ret = sendto(sockfd, packet, packet_len, 0, 
            reinterpret_cast<sockaddr*>(&dest), sizeof(sockaddr_in));
    if(ret == -1){
        errors::SysRet("Fail to send SYN packet");
        res = false; 
    }
    return res;
}

}
