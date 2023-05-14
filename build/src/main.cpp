#include "../../gui/src/gui.h"
#include "../../srcs/include/errors.h"
#include "../../srcs/include/mac.h"
#include "../../srcs/include/ip.h"
#include "../../srcs/include/host_addr.h"
#include "../../srcs/include/scheduler.h"
#include "../../srcs/include/port_scanner.h"
#include <pthread.h>
/*
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
    
    NetNode own_node;
    own_node.ipv4_address = inet_ntoa(ip.sin_addr);
    own_node.mac_address = MAC::mac_to_string(ownmac);
    own_node.vendor = MAC::get_vendor(ownmac);
    own_node.type = "Own host";
    own_node.name = host_addr::get_own_name();
    std::set<std::string> ip_set = IP::all_net_ipv4(net, 0, IP::ip_amount(mask_prefix));
    ip_set.erase(inet_ntoa(ip.sin_addr));
    NodesManager manager;
    manager.SetIps(ip_set);
    manager.SetInterface(interface);
    manager.AddNode(own_node);
    return 0;
}
*/

void* scan_thr_f(void*d){EventSelector* sel = reinterpret_cast<EventSelector*>(d); sel->StartSelecting();return 0;}

int main()
{
    EventSelector selector;
    NodesManager manager; 
    Scheduler* schedule = new Scheduler(selector, manager);
    pthread_t scan_thread;
    pthread_create(&scan_thread, 0, scan_thr_f, &selector); 
    schedule->SetThreadId(scan_thread);
    NetGuardUserInterface n(schedule);
    n.show();
    Fl::run();
    pthread_join(scan_thread, 0);
    return 0;
}
