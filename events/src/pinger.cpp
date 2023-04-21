#include "pinger.h"
#include "../../net/src/ip.h"

Pinger::Pinger(Scheduler& m, const std::string& own_ip, 
    const std::string& net, short mask_prefix) 
    : NetAct(m), ping_ips(0), id(ICMP::get_id()), current_ip_index(0)
{
    ping_interval_size = IP::ip_amount(mask_prefix); 
    ping_ips = IP::all_net_ipv4(net.c_str(), 0, ping_interval_size);
    poll_size = 8; // TODO:make it adaptive to size of network
    
    send_icmp_sd= new int[poll_size];
    recieve_icmp_sd = new int[poll_size];
    send_handlers = new SendEcho*[poll_size]; 
    recieve_hanlders = new RecvEcho*[poll_size];
}

Pinger::~Pinger()
{
    delete[] send_handlers;
    delete[] recieve_hanlders;
    delete[] send_icmp_sd;
    delete[] recieve_icmp_sd;
    delete[] ping_ips;
}

void Pinger::Act()
{
   //first check if we can create new send/echo 
   //no, than skeep
}

