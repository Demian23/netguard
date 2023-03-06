#include "dicmp.hpp"
#include "dneterr.hpp"
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>

namespace DICMP{
    uint16_t calc_checksum(uint16_t *addr, int len);
};
bool DICMP::make_icmp_socket(int &sockfd)
{
    int res = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(res == -1){
        //errors processing
        DERR::SysRet("create socket icmp error, DICMP::make_icmp_socket");
        return false;
    } else {
        sockfd = res;
        return true;
    }
}

void DICMP::send_echo(int sockfd, int id, int seq, struct 
    sockaddr_in *dest_addr, int addr_len)
{
    if(dest_addr == 0 || addr_len == 0)
        DERR::Quit("null dest addr, DICMP::send_echo");
    struct icmp request;
    request.icmp_type = ICMP_ECHO;
    request.icmp_code = 0;
    request.icmp_id = id;
    request.icmp_seq = seq;
    request.icmp_cksum = 0;
    request.icmp_cksum = calc_checksum(
        reinterpret_cast<uint16_t *>(&request), sizeof(request));
    //here request without time 
    sendto(sockfd, reinterpret_cast<void *>(&request), sizeof(request),0, 
        reinterpret_cast<struct sockaddr *>(dest_addr), addr_len);
}

uint16_t DICMP::calc_checksum(uint16_t *addr, int len)
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

ssize_t DICMP::recv_reply(int sockfd, struct msghdr *msg)
{
    enum{icmp_buff_size = 1024};
    char *msg_buf = new char[icmp_buff_size];
    char *controll_buf = new char[icmp_buff_size];

    struct iovec *iovec_s = new struct iovec;
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
            DERR::Sys("recvmsg mistake, DICMP::recv_reply");
    }
    return res;
}

bool DICMP::process_reply(int sockfd, char *ptr, ssize_t len, int id)
{
    int hlen, icmplen;
    struct ip *ip;
    struct icmp *icmp;
    ip = reinterpret_cast<struct ip *>(ptr);
    hlen = ip->ip_hl << 2;
    if(ip->ip_p != IPPROTO_ICMP){
        DERR::Msg("not ICMP, DICMP::process_reply");
        return false;
    }

    icmp = reinterpret_cast<struct icmp *>(ptr + hlen);
    if((icmplen = len - hlen) < 8){
        DERR::Msg("bad packet, DICMP::process_reply");
        return false;
    }
    if(icmp->icmp_type == ICMP_ECHOREPLY){
        if(icmp->icmp_id != id){
//            sendmsg(sockfd, (struct msghdr *)ptr, 0);
            DERR::Msg("wrong id. actual: %d  expected: %d, DICMP::process_reply", icmp->icmp_id, id);
            return false;
        }
        return true;
    }
    DERR::Msg("not ECHO_REPLY, DICMP::process_reply");
    return false;
}
