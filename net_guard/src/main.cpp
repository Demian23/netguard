#include <cstdio>
#include <cstdlib>
#include "../../aux/src/dneterr.hpp"
#include "../../events/src/arp_handler.hpp"
#include "../../aux/src/dip.hpp"
#include "../../aux/src/darp.hpp"
#include "../../aux/src/dmac.hpp"
#include "../../authentication/src/license.hpp"
#include "modules/findip.hpp"

void print_vendor_info(const ether_addr *mac)
{
    DMAC::mac_vendor *vendor = DMAC::vendors_arr(mac, 1);
    if(vendor->find)
        printf("Vendor name: %s\nPrivate: %s\nBlock type: %s\nLast update: %s\n", 
            vendor->info[0],vendor->info[1], vendor->info[2], vendor->info[3]);
    delete[] vendor;
}
void print_info(const sockaddr_in *ip, const ether_addr *mac)
{
    char *ip_s = inet_ntoa(ip->sin_addr);
    char hostname[NI_MAXHOST] = {};
    getnameinfo(reinterpret_cast<const sockaddr *>(ip), sizeof(sockaddr_in), hostname, NI_MAXHOST, 0, 0, 0);
    printf("Device name: %s\n", hostname);
    printf("IP: %s\nMAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
            ip_s, mac->octet[0], mac->octet[1], mac->octet[2], 
            mac->octet[3], mac->octet[4], mac->octet[5]);
    print_vendor_info(mac);
    putchar('\n');
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
    if(argc == 3){
        int new_mask_prefix = atoi(argv[2]);
        if((new_mask_prefix < mask_prefix) || (new_mask_prefix > 32))
            DERR::Quit("Your mask_prefix is wrong for this net with mask %d", mask_prefix);
        mask_prefix = new_mask_prefix;
    }
    if(mask_prefix == 0)
        DERR::Quit("Wrong input, mask must be in prefix form");
    if(net == 0)
        DERR::Quit("Wrong ip address");
}

bool skip_own_ip(const sockaddr_in &ip, const char *dest_ip)
{
    char *ip_s = inet_ntoa(ip.sin_addr);
    return strcmp(ip_s, dest_ip) == 0;
}

int main(int argc, char **argv)
{
    short mask_prefix = 0;
    char *net = 0;
    int len;
    struct ether_addr ownmac;
    struct sockaddr_in ip, mask;
    if(License::is_valid_device(argv[1]))
        printf("Successful authentication\n\n");
    else
        DERR::Quit("Unsuccessful authentication");
    get_cmdl_args(argc, argv, ownmac, ip, mask, mask_prefix, net);
    char **arp_ips = get_real_ip(net, mask_prefix, len);
    print_info(&ip, &ownmac);
    EventSelector ev;
    for(int i = 0; i < len; i++){
        if(skip_own_ip(ip, arp_ips[i]))
            continue;
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
