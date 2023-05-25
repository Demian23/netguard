#include "../include/raw_packets.h"
#include "../include/errors.h"
#include <sys/socket.h>

namespace raw_packets{

struct pseudo_header    //needed for checksum calculation in syn
{
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t tcp_length;
     
    tcphdr tcp;
};

bool make_manual_socket(int& sd, int type)
{
    bool res = false;
    if(make_raw_socket(sd, type)){
        int opt = 1;
        ssize_t ret = setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &opt, 
            sizeof(int));
        if(ret != -1)
            res = true;
    }
    return res;
}

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
int prepare_tcp_packet(char* packet, sockaddr_in* src, sockaddr_in* dest, uint8_t flags)
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
    ip_hdr->ip_src = src->sin_addr;
    ip_hdr->ip_dst = dest->sin_addr;
    ip_hdr->ip_sum = calc_checksum(reinterpret_cast<uint16_t*>(packet), 
        ip_hdr->ip_len >> 1);

    tcp_hdr->th_sport = src->sin_port; 
    tcp_hdr->th_dport = dest->sin_port;
    tcp_hdr->th_seq = 0;
    tcp_hdr->th_ack = 0;
    tcp_hdr->th_off = 5;
    tcp_hdr->th_flags = flags;
    tcp_hdr->th_win = htons(1460);
    tcp_hdr->th_sum = 0;
    tcp_hdr->th_urp = 0;

    psh.source_address = src->sin_addr.s_addr;
    psh.dest_address = dest->sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(20);

    memcpy(&psh.tcp, tcp_hdr, sizeof(tcphdr));
    tcp_hdr->th_sum = calc_checksum(reinterpret_cast<uint16_t*>(&psh), 
        sizeof(pseudo_header));
    return ip_hdr->ip_len;
}

// with manual socket only
bool send_tcp_flag(int sockfd, sockaddr_in* src, sockaddr_in* dest,
    uint8_t flags, char* packet_buffer)
{
    bool res = true;
    int packet_len = prepare_tcp_packet(packet_buffer, src, 
        dest, flags);
    ssize_t ret = sendto(sockfd, packet_buffer, packet_len, 0, 
        reinterpret_cast<sockaddr*>(&dest), sizeof(sockaddr_in));
    if(ret == -1){
        if(errno == ENETUNREACH || errno == EHOSTUNREACH || errno == EHOSTDOWN || errno == ENETDOWN)
            res = false; 
        errors::SysRet("Fail to send SYN packet");
    }
    return res;
}

int id(getpid() & 0xFFFF);
int get_id(){return ++id;}

//check ret value
void send_echo(int sockfd, int id, int seq, 
    sockaddr_in *dest_addr, int addr_len)
{
    icmp request;
    request.icmp_type = ICMP_ECHO;
    request.icmp_code = 0;
    request.icmp_id = id;
    request.icmp_seq = seq;
    request.icmp_cksum = 0;
    request.icmp_cksum = calc_checksum(
        reinterpret_cast<uint16_t *>(&request), sizeof(request));
    sendto(sockfd, reinterpret_cast<void *>(&request), sizeof(request),0, 
        reinterpret_cast<sockaddr *>(dest_addr), addr_len);
}

ssize_t recieve_packet(int sockfd, char* buffer, int buffer_size, 
    sockaddr_in* src)
{
    socklen_t len = sizeof(sockaddr_in);
    ssize_t res = recvfrom(sockfd, buffer, buffer_size, 0, 
        reinterpret_cast<sockaddr*>(src), &len);
    if(res == -1){
        if(errno != EAGAIN)
            errors::SysRet("");
    }
    return res;
}

icmp* is_icmp_msg(char* msg, int size)
{
    int hlen;
    ip *ip;
    icmp *icmp = 0;
    ip = reinterpret_cast<struct ip *>(msg);
    hlen = ip->ip_hl << 2;
    bool bad_packet_size = size - hlen < 8;
    if(ip->ip_p == IPPROTO_ICMP && !bad_packet_size){
        icmp = reinterpret_cast<struct icmp *>(msg + hlen);
    }else{
        if(bad_packet_size){
            errors::Msg("Bad packet size");
        } else 
            errors::Msg("not ICMP");
    }
    return icmp;
}

bool get_echo(char *ptr, ssize_t len, in_addr& src_ip)
{
    bool res = false;
    icmp* icmp_packet = is_icmp_msg(ptr, len);
    if(icmp_packet != 0){
        if(icmp_packet->icmp_type == ICMP_ECHOREPLY){
            ip* ip_packet = reinterpret_cast<ip *>(ptr);
            src_ip = ip_packet->ip_src;
            res = true;
        } else errors::Msg("not echo reply");
    } else errors::Msg("not icmp packet");
    return res;
}

bool send_ttl(int& resfd, int& id, int ttl)
{
    static const char*const dest_str = "8.8.8.8";
    int ret_val, sockfd;
    bool res;
    sockaddr_in dest = {.sin_family = AF_INET};
    id = id == -1 ? get_id() : id;
    res = make_raw_socket(sockfd, IPPROTO_ICMP);
    timeval timeout = {.tv_sec = 1, .tv_usec = 0};
    if(res){
        int opt = ttl;
        ret_val = setsockopt(sockfd, IPPROTO_IP, IP_TTL, &opt, sizeof(opt));
        if(ret_val != -1){
            ret_val = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
            if(ret_val != -1){
                inet_aton(dest_str, &dest.sin_addr); 
                send_echo(sockfd, id, 0, &dest, sizeof(dest));
                resfd = sockfd;
            } else {errors::SysRet("send_ttl, timeout");}
        } else {
            errors::Sys("send_ttl");
        }
    }
    return res; 
}

bool get_exceed_node(int fd, char* ptr, ssize_t len, int id, in_addr& res_addr)
{
    bool res = false;
    icmp* icmp_packet = is_icmp_msg(ptr, len);
    if(icmp_packet != 0){
        if(icmp_packet->icmp_type == ICMP_TIMXCEED){
            ip* ip_packet = reinterpret_cast<struct ip *>(ptr);
            res_addr = ip_packet->ip_src;
            res = true;
        } else {
            errors::Msg("not ECHO_REPLY");
        }
    } 
    return res;
}
}

