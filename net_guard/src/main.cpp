#include <cstdio>
#include <cstdlib>
#include "../../aux/src/dneterr.hpp"
#include "../../events/src/arp_handler.hpp"
#include "../../aux/src/dip.hpp"
#include "../../aux/src/darp.hpp"
#include "findip.hpp"


void print_info(const sockaddr_in *ip, const ether_addr *mac)
{
    char *ip_s = inet_ntoa(ip->sin_addr);
    struct hostent *hp;     
    hp = gethostbyaddr((void*)&ip->sin_addr, sizeof(struct in_addr), AF_INET); 
    /*
     * not working
    if(!getaddrinfo(ip_s, NULL, &hints, &info))
        name = info->ai_canonname;
    else
        name = "";
*/
    printf("%16s\t%s\t%02x:%02x:%02x:%02x:%02x:%02x\n",
            ip_s, hp == NULL ? "none" : hp->h_name,
            mac->octet[0], mac->octet[1], mac->octet[2], 
            mac->octet[3], mac->octet[4], mac->octet[5]);
}

void get_cmdl_args(int argc, char **argv, ether_addr &ownmac, sockaddr_in &ip,
    sockaddr_in &mask, short &mask_prefix, char *&net)
{
    char ip_s[32] = {};
    if(argc > 1 && argc < 4){
        if(DNET::findownaddr(argv[1], &ownmac, &ip, &mask)){
            char *temp = 0;
            temp = inet_ntoa(ip.sin_addr);
            memcpy(ip_s, temp, strlen(temp));
            temp = inet_ntoa(mask.sin_addr);
            mask_prefix = DIP::mask_prefix(temp); 
            net = DIP::ipv4_net(ip_s, mask_prefix); 
        } else {
            DERR::Quit("Can't find interface");
        }
    }else{
        DERR::Quit("usage: %s <interface> [mask_prefix]", argv[0]); 
    }
    if(argc == 3)
        mask_prefix = atoi(argv[2]);
    if(mask_prefix == 0)
        DERR::Quit("Wrong input, mask must be in prefix form");
    if(net == 0)
        DERR::Quit("Wrong ip address");
}

int main(int argc, char **argv)
{
    short mask_prefix = 0;
    char *net = 0;
    int len;
    struct ether_addr ownmac;
    struct sockaddr_in ip, mask;
    get_cmdl_args(argc, argv, ownmac, ip, mask, mask_prefix, net);
    char **arp_ips = get_real_ip(net, mask_prefix, len);
    print_info(&ip, &ownmac);
    EventSelector ev;
    for(int i = 0; i < len; i++){
        ARPHandler *arp_h = new ARPHandler(ip, ownmac, arp_ips + i, 
            1, argv[1], ev);
        arp_h->SetEvents(FdHandler::Timeout + FdHandler::ErrEvent);
        ev.Add(arp_h);
        ev.Run(1);
        const DARP::arp_pair *p = arp_h->GetPairs();
        print_info(&(p->ip), &(p->mac));
        arp_h->ExplicitlyEnd();
    }
    return 0;
}
