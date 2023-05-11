#include "../../gui/src/gui.h"

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
    EventSelector selector;
    Scheduler* scheduler = new Scheduler(selector, manager);
    Pinger* pinger = new Pinger(*scheduler, ip_set);
    Arper* arper = new Arper(*scheduler);
    FindGate* gate = new FindGate(*scheduler);
    PortScanner* scanner = new PortScanner(*scheduler, "192.168.1.15", "192.168.1.1");
    scheduler->AddOrdinaryTask(pinger);
    scheduler->AddOrdinaryTask(arper);
    scheduler->AddOrdinaryTask(gate);
    scheduler->AddOrdinaryTask(scanner);
    selector.AddEvent(scheduler);
    selector.StartSelecting(100);
    std::for_each(manager.GetMap().begin(), manager.GetMap().end(),
        [](std::pair<std::string, NetNode> n){
            printf("Ip: %s, mac: %s, type: %s", n.first.c_str(), 
            n.second.mac_address.c_str(), n.second.type.c_str());
            if(!n.second.name.empty())
                printf(", name: %s", n.second.name.c_str());
            if(!n.second.vendor.empty())
                printf(", vendor: %s", n.second.vendor.c_str());
            if(!n.second.ports.empty()){
                printf("here");
            }
            putchar('\n');
        });
    return 0;
}
*/

int main()
{
    NetGuardUserInterface n; 
    n.show();
    return Fl::run();
}

