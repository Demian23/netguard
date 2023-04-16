#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>

#include "icmp.h"
#include "errors.h"

namespace ICMP{

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
        errors::Sys("recvmsg mistake");
    }
    return res;
}

bool process_reply(int sockfd, char *ptr, ssize_t len, int id)
{
    int hlen, icmplen;
    struct ip *ip;
    struct icmp *icmp;
    ip = reinterpret_cast<struct ip *>(ptr);
    hlen = ip->ip_hl << 2;
    if(ip->ip_p != IPPROTO_ICMP){
        errors::Msg("not ICMP, DICMP::process_reply");
        return false;
    }

    icmp = reinterpret_cast<struct icmp *>(ptr + hlen);
    if((icmplen = len - hlen) < 8){
        errors::Msg("bad packet, DICMP::process_reply");
        return false;
    }
    if(icmp->icmp_type == ICMP_ECHOREPLY){
        if(icmp->icmp_id != id){
//            sendmsg(sockfd, (struct msghdr *)ptr, 0);
            //DERR::Msg("wrong id. actual: %d  expected: %d, DICMP::process_reply", icmp->icmp_id, id);
            return false;
        }
        return true;
    }
    errors::Msg("not ECHO_REPLY, DICMP::process_reply");
    return false;
}

};
