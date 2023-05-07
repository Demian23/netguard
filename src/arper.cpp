#include "../include/arper.h"

#include "../include/arp.h"
Arper::Arper(Scheduler& m) : master(m){}

bool Arper::Execute()
{
    std::vector<NetDevice>& updated_dev = master.GetDevStat().devices;
    const char*const interface = master.GetDevStat().interface.c_str();
    std::vector<NetDevice>::iterator own_host_iterator 
        = std::find_if(updated_dev.begin(), updated_dev.end(), 
        [](NetDevice d){return d.GetType() == OwnHost;});
    sockaddr_in own_ip;
    ether_addr own_mac;
    own_host_iterator->GetIp(own_ip);    
    own_host_iterator->GetMac(own_mac);
    int fd, buffer_length;
     ARP::set_bpf_arp(fd, buffer_length, interface);
    char *bpf_buffer = new char[buffer_length];
    for(int i = 0; i < updated_dev.size(); i++)
        if(!updated_dev[i].HasMac()){
            int counter = 0; bool find;
            ARP::arp_pair p;
            do{
                ARP::writequery(fd, &own_mac, &own_ip, updated_dev[i].GetIp().c_str());
                find = ARP::collectresponse(fd, p, bpf_buffer, buffer_length) 
                && updated_dev[i].GetIp() == inet_ntoa(p.ip.sin_addr);
                counter++;
            }while(!find && counter < 5);
            if(find)
                updated_dev[i].SetMac(p.mac);
        }
    delete[] bpf_buffer;
    return true;
}
