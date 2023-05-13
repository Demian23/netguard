#include "../include/arper.h"
#include "../include/arp.h"
#include <arpa/inet.h>
#include <cstring>
#include "../include/mac.h"
#include "../include/ip.h"
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
    NetMap& map = master.manager.GetMap();
    for(NetMap::iterator it = map.begin(); it != map.end(); it++){
        if(it->second.mac_address.empty()){
            int counter = 0; bool find;
            ARP::arp_pair p;
            do{
                ARP::writequery(fd, &own_mac, &own_ip, it->first.c_str());
                find = ARP::collectresponse(fd, p, bpf_buffer, buffer_length);
                find &= it->first == inet_ntoa(p.ip.sin_addr);
                counter++;
            }while(!find && counter < 5);
            if(find){
                it->second.mac_address = MAC::mac_to_string(p.mac);
                it->second.vendor = MAC::get_vendor(p.mac);
            }
        }
    }
    delete[] bpf_buffer;
    return true;
}