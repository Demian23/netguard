#include "dnet.hpp"
#include "dneterr.hpp"
#include <string.h>
#include <ifaddrs.h>
#include <netinet/if_ether.h>
#include <net/if_types.h>
#include <net/if_dl.h>

struct sockaddr_in DNET::set_addr(const char *ip, int family)
{
    if(ip == 0)
        DERR::Msg("null poinetr ip, DNET::set_addr");
    struct sockaddr_in res;
    res.sin_family = family;  
    if(inet_aton(ip, &res.sin_addr) == 0)
        DERR::Quit("invalid address");
    return res;
}

char* DNET::get_local_ip()
{
    struct ifaddrs *interface = new struct ifaddrs;
    char *res = 0;
    if(getifaddrs(&interface) != 0)
        return 0;
    struct ifaddrs *temp = interface;
    while(temp){
        if(temp->ifa_addr && temp->ifa_addr->sa_family == AF_INET){
            struct sockaddr_in *paddr = reinterpret_cast<struct sockaddr_in *>(temp->ifa_addr);
            res = inet_ntoa(paddr->sin_addr);
            if(strncmp(res, "127", 3) != 0){
                break; 
            }
        }
        temp = temp->ifa_next;
    }
    freeifaddrs(interface); 
    return res;
}

bool DNET::findownaddr(const char *interface, struct ether_addr *ownmac, 
    struct sockaddr_in *saip, struct sockaddr_in *samask)
{
    struct ifaddrs *ifa, *temp;
    struct sockaddr_dl *sdl;
    bool some_interface = interface == 0;
    const char *loop = "lo";
    bool success[2] = {};

    if(!getifaddrs(&ifa)){
        for(temp = ifa; temp; temp = temp->ifa_next){
            if(some_interface){
                interface = strncmp(loop, temp->ifa_name, 2) == 0 ? "l" : temp->ifa_name; 
            }
            if(!strcmp(temp->ifa_name, interface)){
                sdl = reinterpret_cast<sockaddr_dl *>(temp->ifa_addr); 
                if(sdl->sdl_family == AF_LINK && sdl->sdl_type == IFT_ETHER
                    && sdl->sdl_alen == ETHER_ADDR_LEN){
                    memcpy(reinterpret_cast<uint8_t *>(ownmac), 
                        reinterpret_cast<uint8_t *>(LLADDR(sdl)), 
                        sizeof(struct ether_addr));
                    success[0] = true;
                } else {
                    if(sdl->sdl_family == AF_INET){
                        saip->sin_addr.s_addr = ((struct sockaddr_in *)temp->ifa_addr)->sin_addr.s_addr;
                        samask->sin_addr.s_addr = ((struct sockaddr_in *)temp->ifa_netmask)->sin_addr.s_addr;
                        success[1] = true;
                    }
                }
            }
            if(success[0] && success[1])
                break;
        }
        freeifaddrs(ifa);
        return success[1] && success[0];
    } else {
        DERR::Msg("Get own address mistake, DNET::findownaddr");
        return false;
    }
}
