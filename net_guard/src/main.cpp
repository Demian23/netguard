/*
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <unistd.h>
#include <netdb.h>

#include "../../net/src/ip.h"
#include "../../net/src/arp.h"
#include "../../net/src/mac.h"

#include "../../net/src/errors.h"
#include "../../events/src/arp_handler.h"
#include "../../events/src/icmp_handlers.h"
#include "../../authentication/src/license.hpp"
#include "../../net/src/host_addr.h"

static ether_addr not_found;

int poll_ip_size(const int interval_size)
{
    int res = 1;
    if(interval_size > 0x0F)
        res <<= 1;
    if(interval_size > 0x80)
        res <<= 1;
    if(interval_size > 0xFF)
        res <<= 1;
    return res;
}
std::vector<std::string> ping_net(const std::string& net, 
    const short mask_prefix, const std::string& own_ip)
{
    int id = getpid() & 0xFFFF;
    int interval_size = IP::ip_amount(mask_prefix);
    char **ip_array = IP::all_net_ipv4(net.c_str(), 0, interval_size);
    const int poll_size = poll_ip_size(interval_size);
    EventSelector selector;
    std::vector<std::string> res;
    
    int* send_icmp_sd= new int[poll_size];
    int* recieve_icmp_sd = new int[poll_size];
    SendEcho** send_handlers = new SendEcho*[poll_size]; 
    RecvEcho** recieve_hanlders = new RecvEcho*[poll_size];

    for(int i = 0; i < interval_size; i += poll_size){
        for(int j = 0; j < poll_size && i + j < interval_size; j++){
            if(own_ip == ip_array[i + j])
                continue;
            if(!ICMP::make_icmp_socket(send_icmp_sd[j])
            || !ICMP::make_icmp_socket(recieve_icmp_sd[j]))
                errors::Sys("ICMP socket not created.");
            send_handlers[j] = new SendEcho(send_icmp_sd[j], true, 
                ip_array[i + j], selector, id);
            recieve_hanlders[j] = new RecvEcho(recieve_icmp_sd[j], true, id);

            send_handlers[j]->SetEvents(FdHandler::Timeout + FdHandler::ErrEvent);
            recieve_hanlders[j]->SetEvents(FdHandler::InEvent + FdHandler::ErrEvent);
            
            selector.Add(send_handlers[j]); selector.Add(recieve_hanlders[j]);
            id++;
        }
        selector.Run(200);
        for(int j = 0; j < poll_size && i + j < interval_size; j++){
            if(recieve_hanlders[j]->Exists())
                res.push_back(ip_array[i + j]);

            send_handlers[j]->SetEvents(FdHandler::None);
            selector.UpdateEvents(send_handlers[j]);
            send_handlers[j]->ExplicitlyEnd();

            recieve_hanlders[j]->SetEvents(FdHandler::None);
            selector.UpdateEvents(recieve_hanlders[j]);
            recieve_hanlders[j]->ExplicitlyEnd();

            delete[] ip_array[i + j];
        }
    }
    setuid(getuid());
    delete[] send_handlers;
    delete[] recieve_hanlders;
    delete[] send_icmp_sd;
    delete[] recieve_icmp_sd;
    delete[] ip_array;

    return res;
}

std::vector<ARP::arp_pair> ip_to_mac(const sockaddr_in& own_ip, const ether_addr& own_mac, 
        const std::vector<std::string> arp_ip, const std::string& interface)
{
    EventSelector selector;
    std::vector<ARP::arp_pair> res;
    res.push_back(ARP::arp_pair(own_mac, own_ip));
    for(int i = 0; i < arp_ip.size(); i ++){
        ARPHandler arp_h(own_ip, own_mac, arp_ip[i], 
                interface, selector);
        arp_h.HandleTimeout(); 
        ARP::arp_pair p;
        if(arp_h.Find())
            p = arp_h.GetPair();
        else{
            p.ip = host_addr::set_addr(arp_ip[i].c_str(), AF_INET);
            p.mac = not_found;
        };
        res.push_back(p);
    }
    return res;
}

void get_cmdl_args(int argc, char **argv, std::string& interface, ether_addr &ownmac, sockaddr_in &ip,
        sockaddr_in &mask, short &mask_prefix, std::string& net)
{
    enum{cmdl_argc = 3};
    if(argc == cmdl_argc){
        interface = argv[1];
        bool interface_exist = host_addr::findownaddr(interface, ownmac, ip, mask);
        if(interface_exist){
            int usr_mask_prefix = atoi(argv[2]);
            std::string mask_string = inet_ntoa(mask.sin_addr);
            mask_prefix = IP::mask_prefix(mask_string.c_str());
            net = inet_ntoa(ip.sin_addr);
            if(usr_mask_prefix < IP::max_mask_prefix && usr_mask_prefix >= mask_prefix)
                net = IP::ipv4_net(net.c_str(), usr_mask_prefix);
            else
                errors::Quit("Wrong mask for this net."
                   " Should be greater than %d.", mask_prefix);
       } else 
            errors::Quit("Interface don't exists.");
    } else 
        errors::Quit("usage: %s <interface> <mask_prefix>", argv[0]); 
}

void print_res_info(const ARP::arp_pair& p)
{
    std::string ip = inet_ntoa(p.ip.sin_addr);
    char hostname[NI_MAXHOST] = {};
    getnameinfo(reinterpret_cast<const sockaddr *>(&p.ip), sizeof(sockaddr_in), 
            hostname, NI_MAXHOST, 0, 0, 0);
    printf("Hostname: %s\nIP: %s\n", hostname, ip.c_str());
    if(memcmp(&p.mac, &not_found, sizeof(p.mac)) != 0){
        printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",p.mac.octet[0], 
            p.mac.octet[1], p.mac.octet[2], p.mac.octet[3], p.mac.octet[4], p.mac.octet[5]);
        MAC::mac_vendor vendor = MAC::vendor(p.mac);
        IP::DevType type = IP::endpoint;
        if(vendor.find){
            printf("Vendor name: %s\nPrivate: %s\nBlock type: %s\nLast update: %s\n", 
                    vendor.info[0],vendor.info[1], vendor.info[2], vendor.info[3]);
            type = IP::devtype_from_vendor(vendor.info[0]);
        }
        if(type == IP::endpoint)
            printf("Device type: endpoint\n");
        else
            printf("Device type: customer premise equipment\n");
    } else {
        printf("MAC: Not found.\n");
    }
    putchar('\n');
}
*/

#include "../../events/src/scheduler.h"
#include "../../events/src/pinger.h"
#include "../../net/src/host_addr.h"
#include "../../net/src/ip.h"
#include "../../net/src/errors.h"


void get_cmdl_args(int argc, char **argv, std::string& interface, ether_addr &ownmac, sockaddr_in &ip,
        sockaddr_in &mask, short &mask_prefix, std::string& net)
{
    enum{cmdl_argc = 3};
    if(argc == cmdl_argc){
        interface = argv[1];
        bool interface_exist = host_addr::findownaddr(interface, ownmac, ip, mask);
        if(interface_exist){
            int usr_mask_prefix = atoi(argv[2]);
            mask_prefix = IP::mask_prefix(inet_ntoa(mask.sin_addr));
            net = inet_ntoa(ip.sin_addr);
            if(usr_mask_prefix < IP::max_mask_prefix && usr_mask_prefix >= mask_prefix)
                net = IP::ipv4_net(net, usr_mask_prefix);
            else
                errors::Quit("Wrong mask for this net."
                   " Should be greater than %d.", mask_prefix);
       } else 
            errors::Quit("Interface don't exists.");
    } else 
        errors::Quit("usage: %s <interface> <mask_prefix>", argv[0]); 
}

void print(const NetDevice& dev)
{
    const dev_info& info = dev.GetInfo();
    dev_info::const_iterator it = info.begin();
    for(;it != info.end(); it++){
        printf("%s: %s\n", it->first.c_str(), it->second.c_str());
    }
    putchar('\n');
}

int main(int argc, char **argv)
{
    short mask_prefix;
    ether_addr ownmac;
    sockaddr_in ip, mask;
    std::string interface, net;
    get_cmdl_args(argc, argv, interface, ownmac, ip, mask, mask_prefix, net);
    
    NetDevice own_device; 
    own_device.SetIpv4(ip.sin_addr);
    own_device.SetMac(ownmac);
    std::vector<NetDevice> devices;
    devices.push_back(own_device);
    
    EventSelector selector;
    Scheduler* scheduler = new Scheduler(2, selector, devices);
    std::set<std::string> ip_set = IP::all_net_ipv4(net, 0, IP::ip_amount(mask_prefix));
    ip_set.erase(inet_ntoa(ip.sin_addr));
    Pinger* ping = new Pinger(*scheduler, ip_set);
    scheduler->SetNormalScheduledEvent(ping);
    selector.Add(scheduler);
    selector.Run(1000);
    std::for_each(devices.begin(), devices.end(), print); 
    return 0;
}
