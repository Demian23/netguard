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
    ARP::ip_mac_map map;
    for(std::string ip : ips){
        short counter = 0;
        do{
            ARP::writequery(fd, &own_mac, &own_ip, ip.c_str());
            counter++;
        }while(!ARP::collectresponse(fd, map, bpf_buffer, buffer_length) && counter < 3);
    }
    for(const auto& el : map){
        NetNode* temp = master.manager.GetNodeByIp(el.first);
        if(temp && !el.second.empty()){
            temp->mac_address = el.second;
            ether_addr* mac = ether_aton(temp->mac_address.c_str());
            temp->vendor = MAC::get_vendor(*mac);
        }
    }
    master.manager.Change();
    delete[] bpf_buffer;
    close(fd);
    return true;
}
