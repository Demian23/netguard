#include "../include/arper.h"
#include "../include/arp.h"
#include <arpa/inet.h>
#include <cstring>
#include "../include/mac.h"
#include "../include/ip.h"
#include <unistd.h>
Arper::Arper(Scheduler& m) : master(m){}

bool Arper::Execute()
{
    const std::string interface = master.manager.GetInterface();
    sockaddr_in own_ip;
    ether_addr own_mac;
    inet_aton(master.manager.GetOwnNode().ipv4_address.c_str(), &own_ip.sin_addr);
    ether_addr* temp_mac = ether_aton(master.manager.GetOwnNode().mac_address.c_str());
    memmove(&own_mac, temp_mac, ETHER_ADDR_LEN);
    int fd, buffer_length;
    ARP::set_bpf_arp(fd, buffer_length, interface.c_str());
    char *bpf_buffer = new char[buffer_length];
    std::vector<std::string> ips = master.manager.GetIps(); 
    for(std::string ip : ips){
        NetNode* temp = master.manager.GetNodeByIp(ip);
        if(temp->mac_address.empty()){
            int counter = 0; bool find;
            ARP::arp_pair p;
            do{
                ARP::writequery(fd, &own_mac, &own_ip, ip.c_str());
                find = ARP::collectresponse(fd, p, bpf_buffer, buffer_length);
                find &= ip == inet_ntoa(p.ip.sin_addr);
                counter++;
            }while(!find && counter < 5);
            if(find){
                temp->mac_address = MAC::mac_to_string(p.mac);
                temp->vendor = MAC::get_vendor(p.mac);
            }
        }
    }
    master.manager.Change();
    delete[] bpf_buffer;
    close(fd);
    return true;
}
