#include "../include/tcp.h"
#include "../include/errors.h"
#include "../include/nodes_manager.h"
#include "../include/raw_packets.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <net/bpf.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <string.h>

bool set_bpf_no_filter(int &fd, int &buflen, const char *interface)
{
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
    timeval timeout = {.tv_sec = 0, .tv_usec = 50000};
    ioctl(fd, BIOCSRTIMEOUT, &timeout);
    return res;
}

//bad decision with passing ports
void get_syn_answer(int fd, int buflen, ports_storage& ports)
{
    int read_bytes = 0;
    ether_header* ethernet_frame;
    ip* ip_header; 
    tcphdr* tcp_header;
    bpf_hdr* bpf_buffer = new bpf_hdr[buflen];
    bpf_hdr* bpf_packet;
    memset(bpf_buffer, 0, buflen);
    if((read_bytes = read(fd, bpf_buffer, buflen)) > 0){
        char* ptr = reinterpret_cast<char*>(bpf_buffer);
        while(ptr < (reinterpret_cast<char*>(bpf_buffer) + read_bytes)){
			bpf_packet = reinterpret_cast<bpf_hdr*>(ptr);
			ethernet_frame = (ether_header*)((char*) bpf_packet + bpf_packet->bh_hdrlen);
            // check dest mac addr
            // check type
            if(ethernet_frame->ether_type == ETHERTYPE_IP){
                ip_header = (ip*)((char*)ethernet_frame + sizeof(ether_header));
                if(ip_header->ip_p == IPPROTO_TCP){
                    tcp_header = (tcphdr*)((char*)ip_header+ sizeof(ip));
                    //here check flags and port and if ok, than add result to ports
                }
                
            }
			ptr += BPF_WORDALIGN(bpf_packet->bh_hdrlen + bpf_packet->bh_caplen);
		}
    } else {
        errors::SysRet("get_syn_answer");
    }
}  
