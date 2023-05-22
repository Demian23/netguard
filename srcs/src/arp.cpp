#include <string>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/bpf.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "../include/arp.h"
#include "../include/errors.h"

namespace ARP{

void set_timeout(int fd)
{
    timeval timeout = {.tv_sec = 0, .tv_usec = 50000};
    ioctl(fd, BIOCSRTIMEOUT, &timeout);
}

void prepareframe(ether_addr *ownmac, sockaddr_in *ip, 
    ether_header *ethhdr, ether_arp *arp)
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
        reinterpret_cast<uint8_t *>(ownmac), sizeof(ether_addr));
    memcpy(reinterpret_cast<uint8_t *>(arp->arp_spa),
        reinterpret_cast<uint8_t *>(&ip->sin_addr.s_addr), 4 * sizeof(uint8_t));
    memset(reinterpret_cast<uint8_t *>(arp->arp_tha), 0, ETHER_ADDR_LEN);
}

void writequery(int fd, ether_addr *ownmac, 
    sockaddr_in *ownip, const char *ip)
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

bool collectresponse(int fd, ip_mac_map &map, char *buffer, int buflen)
{
    int read_bytes;
    bool result = false;
    memset(buffer, 0, buflen);
    if((read_bytes = read(fd, buffer, buflen)) > 0){
        char* ptr = reinterpret_cast<char*>(buffer);
        bpf_hdr* bpf_packet;
        while(ptr < buffer + read_bytes){
            bpf_packet = reinterpret_cast<bpf_hdr*>(ptr);
            if(buffer[bpf_packet->bh_hdrlen + 0x12] == 0x06
            && buffer[bpf_packet->bh_hdrlen + 0x13] == 0x04){
                in_addr addr; ether_addr mac;
                memcpy(&addr, ptr + bpf_packet->bh_hdrlen + 0x1c, sizeof(struct in_addr));
                memcpy((uint8_t *)&mac, ptr+ bpf_packet->bh_hdrlen + 0x16, ETHER_ADDR_LEN);
                std::string addr_string = inet_ntoa(addr);
                if(map.find(addr_string) != map.end()){
                    if(map[addr_string].empty())
                        map[addr_string] = ether_ntoa(&mac);
                }else map.insert(std::make_pair(addr_string, ether_ntoa(&mac)));
                result = true;
            }
            ptr += BPF_WORDALIGN(bpf_packet->bh_hdrlen + bpf_packet->bh_caplen);
        }
    }
    return result;
}

bool set_bpf_arp(int &fd, int &buflen, const char *interface)
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
        errors::Sys("Cant open bpf, DBPF::set_bpf");
    }
    struct ifreq ir;
    strncpy(ir.ifr_name, interface, IFNAMSIZ);
    bool res = ioctl(fd, BIOCSETIF, &ir) != -1 && 
        ioctl(fd, BIOCIMMEDIATE, &buflen) != -1 &&
        ioctl(fd, BIOCGBLEN, &buflen) != -1;
    if(!res)
        errors::Sys("ioctl, DBPF::set_bpf");
    res = ioctl(fd, BIOCGDLT, &dlt) != -1 && dlt == DLT_EN10MB;
    if(!res)
        errors::Sys("not ethernet? DBPF::set_bpf");
    res = ioctl(fd, BIOCSETF, &filter) != -1;
    if(!res)
        errors::Sys("Cant set BPF rule, DBPF::set_bpf");
    set_timeout(fd);
    return res;
}

};
