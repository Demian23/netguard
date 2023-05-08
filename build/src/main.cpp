#include "../../include/scheduler.h"
#include "../../include/pinger.h"
#include "../../include/arper.h"
#include "../../include/router.h"
#include "../../include/host_addr.h"
#include "../../include/ip.h"
#include "../../include/errors.h"
#include <algorithm>


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
    own_device.SetType(OwnHost);
    std::vector<NetDevice> devices;
    devices.push_back(own_device);
    std::set<std::string> ip_set = IP::all_net_ipv4(net, 0, IP::ip_amount(mask_prefix));
    ip_set.erase(inet_ntoa(ip.sin_addr));
    NetInfo inf = {.ip_set = ip_set, .interface = interface, .devices = devices};
    EventSelector selector;
    Scheduler* scheduler = new Scheduler(selector, inf);
    Pinger* pinger = new Pinger(*scheduler, ip_set);
    Arper* arper = new Arper(*scheduler);
    FindGate* gate = new FindGate(*scheduler);
    scheduler->AddOrdinaryTask(pinger);
    scheduler->AddOrdinaryTask(arper);
    scheduler->AddOrdinaryTask(gate);
    selector.AddEvent(scheduler);
    selector.StartSelecting(500);
    std::for_each(devices.begin(), devices.end(), [](NetDevice e){print(e);});
    return 0;
}

