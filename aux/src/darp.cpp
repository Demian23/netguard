#include "darp.hpp"
#include "dneterr.hpp"
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <net/bpf.h>
namespace DARP{
    void prepareframe(struct ether_addr *ownmac, struct sockaddr_in
        *ip, struct ether_header *ethhdr, struct ether_arp *arp);
};

void DARP::writequery(int fd, struct ether_addr *ownmac, 
    struct sockaddr_in *ownip, char *ip)
{
    const int size = sizeof(struct ether_header) +
       sizeof(struct ether_arp);
   unsigned char msg[size];
   struct ether_header *ethh;
   struct ether_arp *arp;
   struct sockaddr_in dest;

   ethh = reinterpret_cast<struct ether_header *>(msg);
   arp = reinterpret_cast<struct ether_arp *>(ethh + 1);

   prepareframe(ownmac, ownip, ethh, arp);

   int len = 0;
   inet_aton(ip, &dest.sin_addr);
   memcpy(reinterpret_cast<uint8_t*>(arp->arp_tpa),
           reinterpret_cast<uint8_t*>(&dest.sin_addr.s_addr), 4 * sizeof(uint8_t));
   int addlen = -1;
   while(len < size && addlen != 0){
       addlen = write(fd, ethh + len, size - len); 
       len += addlen;
   } 
}

void DARP::prepareframe(struct ether_addr *ownmac, struct sockaddr_in *ip, 
    struct ether_header *ethhdr, struct ether_arp *arp)
{
    memset(reinterpret_cast<uint8_t *>(&ethhdr->ether_dhost), 0xFF, ETHER_ADDR_LEN);     
    memcpy(reinterpret_cast<uint8_t *>(&ethhdr->ether_shost),
        reinterpret_cast<uint8_t *>(ownmac), ETHER_ADDR_LEN);
    ethhdr->ether_type = htons(ETHERTYPE_ARP);

    arp->arp_hrd = htons(ARPHRD_ETHER);
    arp->arp_pro = htons(ETHERTYPE_IP);
    arp->arp_hln = ETHER_ADDR_LEN;
    arp->arp_pln = 4;
    arp->arp_op = htons(ARPOP_REQUEST);
    memcpy(reinterpret_cast<uint8_t *>(arp->arp_sha),
        reinterpret_cast<uint8_t *>(ownmac), sizeof(struct ether_addr));
    memcpy(reinterpret_cast<uint8_t *>(arp->arp_spa),
        reinterpret_cast<uint8_t *>(&ip->sin_addr.s_addr), 4 * sizeof(uint8_t));
    memset(reinterpret_cast<uint8_t *>(arp->arp_tha), 0, ETHER_ADDR_LEN);
}

bool DARP::collectresponse(int fd, arp_pair &p, char *buffer, int buflen)
{
    int len;
    struct bpf_hdr *bpf_h = reinterpret_cast<struct bpf_hdr *>(buffer);
    len = read(fd, buffer, buflen);
    if(len >= sizeof(struct bpf_hdr) && len >= bpf_h->bh_hdrlen + 0x2a 
        && buffer[bpf_h->bh_hdrlen + 0x12] == 0x06
        && buffer[bpf_h->bh_hdrlen + 0x13] == 0x04)
    {
        memcpy(&p.ip.sin_addr.s_addr, buffer + bpf_h->bh_hdrlen + 0x1c, sizeof(struct in_addr));
        memcpy((uint8_t *)&p.mac, buffer + bpf_h->bh_hdrlen + 0x16, ETHER_ADDR_LEN);
        return true;
    }
    return false;
}

bool DARP::set_bpf_arp(int &fd, int &buflen, const char *interface)
{
	struct bpf_insn insns[] = {
		// Load word at octet 12
		BPF_STMT(BPF_LD | BPF_H | BPF_ABS, 12),
		// If not ETHERTYPE_ARP, skip next 3 (and return nothing)
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, ETHERTYPE_ARP, 0, 3),
		// Load word at octet 20
		BPF_STMT(BPF_LD | BPF_H | BPF_ABS, 20),
		// If not ARPOP_REPLY, skip next 1 (and return nothing)
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, ARPOP_REPLY, 0, 1),
		// Valid ARP reply received, return message
		BPF_STMT(BPF_RET | BPF_K, sizeof(struct ether_arp) + sizeof(struct ether_header)),
		// Return nothing
		BPF_STMT(BPF_RET | BPF_K, 0),
	};
	struct bpf_program filter = {
		sizeof insns / sizeof(insns[0]),
		insns
	};
    char dev[32] = {};
    buflen = 1;
    int dlt;
    for(int i = 0; i < 255; i++){
        sprintf(dev, "/dev/bpf%d", i);
        fd = open(dev, O_RDWR); 
        if(fd > 0)
            break;
    }
    if(fd < 0){
        DERR::Sys("Cant open bpf, DBPF::set_bpf");
    }
    struct ifreq ir;
    strncpy(ir.ifr_name, interface, IFNAMSIZ);
    bool res = ioctl(fd, BIOCSETIF, &ir) != -1 && 
        ioctl(fd, BIOCIMMEDIATE, &buflen) != -1 &&
        ioctl(fd, BIOCGBLEN, &buflen) != -1;
    if(!res)
        DERR::Sys("ioctl, DBPF::set_bpf");
    res = ioctl(fd, BIOCGDLT, &dlt) != -1 && dlt == DLT_EN10MB;
    if(!res)
        DERR::Sys("not ethernet? DBPF::set_bpf");
    res = ioctl(fd, BIOCSETF, &filter) != -1;
    if(!res)
        DERR::Sys("Cant set BPF rule, DBPF::set_bpf");
    return res;
}

void DARP::set_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void DARP::set_timeout(int fd)
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = arp_timeout;
    ioctl(fd, BIOCSRTIMEOUT, &timeout);
}
