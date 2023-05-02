#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/icmp.h"
#include "../include/errors.h"

namespace ICMP{

int id = getpid();

//this function not multithreading
int get_id()
{
   return ++id & 0xFFFF; 
}

bool make_icmp_socket(int &sockfd)
{
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sockfd == -1){
        errors::SysRet("create socket icmp error, DICMP::make_icmp_socket");
        return false;
    } else 
        return true;
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
    //here request without time 
    // assume that it's about ttl
    sendto(sockfd, reinterpret_cast<void *>(&request), sizeof(request),0, 
        reinterpret_cast<sockaddr *>(dest_addr), addr_len);
}

ssize_t recv_reply(int sockfd, struct msghdr *msg)
{
    enum{icmp_buff_size = 1024};
    char *msg_buf = new char[icmp_buff_size];
    char *controll_buf = new char[icmp_buff_size];

    iovec *iovec_s = new iovec;
    iovec_s->iov_len = icmp_buff_size;
    iovec_s->iov_base = msg_buf;

    msg->msg_name = 0;
    msg->msg_namelen = 0;
    msg->msg_iov = iovec_s;
    msg->msg_iovlen = 1;
    msg->msg_control = controll_buf;
    msg->msg_controllen = icmp_buff_size;
    msg->msg_flags = 0; 
    ssize_t res = recvmsg(sockfd, msg, 0);
    if(res == -1){
        if(errno != EAGAIN)
            errors::Sys("recvmsg mistake");
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

Errors get_echo(char *ptr, ssize_t len, in_addr& src_ip)
{
    Errors res;
    icmp* icmp_packet = is_icmp_msg(ptr, len);
    if(icmp_packet != 0){
        if(icmp_packet->icmp_type == ICMP_ECHOREPLY){
            ip* ip_packet = reinterpret_cast<ip *>(ptr);
            src_ip = ip_packet->ip_src;
            res = Allright;
        } else {
            res = NotEcho;
            errors::Msg("not ECHO_REPLY");
        }
    } else {
        res = NotIcmp;
    }
    return res;
}

bool send_ttl_1(int& resfd, int& id)
{
    static const char*const dest_str = "8.8.8.8";
    int ret_val, sockfd;
    bool res;
    sockaddr_in dest = {.sin_family = AF_INET};
    id = id == -1 ? get_id() : id;
    res = make_icmp_socket(sockfd);
    if(res){
        int opt = 1;
        ret_val = setsockopt(sockfd, IPPROTO_IP, IP_TTL, &opt, sizeof(opt));
        if(ret_val != -1){
            inet_aton(dest_str, &dest.sin_addr); 
            send_echo(sockfd, id, 0, &dest, sizeof(dest));
            resfd = sockfd;
        } else {
            errors::Sys("find gateway");
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



void send_routers_request(int sockfd)
{
    static const char* const router_multicast_str = "224.0.0.2";
    int ret_code;
    icmp request;
    request.icmp_type = ICMP_ROUTERSOLICIT;
    request.icmp_code = 0;
    request.icmp_cksum = 0;
    request.icmp_cksum = calc_checksum(
            reinterpret_cast<uint16_t *>(&request), sizeof(request));
    sockaddr_in router_multicast;
    inet_aton(router_multicast_str, &router_multicast.sin_addr);
    ret_code = sendto(sockfd, reinterpret_cast<void *>(&request), sizeof(request),0, 
        reinterpret_cast<sockaddr *>(&router_multicast), sizeof(router_multicast));
    if(ret_code == -1){
        errors::Sys("");
    }
}

bool send_routers_request_v6(int& newsockfd)
{
    static const char*const router_multicast_v6_str = "FF02::2";
    icmp6_filter router_advert_filter;
    newsockfd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    bool res_flag = true;
    int ret_code = 0;
    if(newsockfd != -1){
        ICMP6_FILTER_SETBLOCKALL(&router_advert_filter);
        ICMP6_FILTER_SETPASS(ND_ROUTER_ADVERT, &router_advert_filter);
        ret_code = setsockopt(newsockfd, IPPROTO_ICMPV6, ICMP6_FILTER, 
            &router_advert_filter, sizeof(router_advert_filter));
        if(ret_code != -1){
            sockaddr_in6 router_multicast_v6;
            ret_code = inet_pton(AF_INET6, router_multicast_v6_str, &router_multicast_v6);
            if(ret_code == 1){
                icmp6_hdr request;
                request.icmp6_type = ND_ROUTER_SOLICIT;
                request.icmp6_code = 0;
                ret_code = sendto(newsockfd, reinterpret_cast<void *>(&request), sizeof(request),0, 
                    reinterpret_cast<sockaddr *>(&router_multicast_v6), sizeof(router_multicast_v6));
                if(ret_code == -1){
                    errors::Sys("");
                    res_flag = false;
                }
            } else 
                if(ret_code == 0){
                    errors::Msg("Wrong address family");
                    res_flag = false;
                } else {
                    errors::Sys("system error.");
                    res_flag = false;
                }
        } else {
            errors::Sys("setting filter error");
            res_flag = false;
        }
    } else {
        errors::Sys("Can't create icmpv6 socket.");
        res_flag = false;
    }
    return res_flag;
}

};
