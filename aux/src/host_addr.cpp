#include <string.h>
#include <ifaddrs.h>
#include <netinet/if_ether.h>
#include <net/if_types.h>
#include <net/if_dl.h>

#include "host_addr.h"
#include "dneterr.hpp"

namespace host_addr{


    sockaddr_in set_addr(const char *ip, int family)
    {
        sockaddr_in res;
        res.sin_family = family;  
        if(!inet_aton(ip, &res.sin_addr))
            DERR::Quit("invalid address");
        return res;
    }

    bool findownaddr(String& interface, ether_addr& ownmac, 
        sockaddr_in& saip, sockaddr_in& samask)
    {
        ifaddrs *ifa, *temp;
        sockaddr_dl *sdl;
        bool no_interface= interface == 0;
        static const char *loop = "lo";
        bool success[2] = {};

    if(getifaddrs(&ifa)){
        DERR::Msg("Get own address mistake, DNET::findownaddr");
        return false;
    }

    for(temp = ifa; temp; temp = temp->ifa_next){
        if(no_interface){
            if(strncmp(loop, temp->ifa_name, 2) == 0)
                continue;
            else
                copy_interface(interface, temp->ifa_name);
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
                    saip->sin_family = AF_INET;
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
}


};


    String host_addr::get_local_ip()
    {
        ifaddrs *interface = new ifaddrs;
        if(getifaddrs(&interface) != 0)
            return String();
        ifaddrs *temp = interface;
        char* res;
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
        return String(res);
    }


void DNET::copy_interface(char *&interface, const char *real_interface)
{
    if(interface)
        delete[] interface;
    int size = strlen(real_interface);
    interface = new char[size + 1];
    strcpy(interface, real_interface);
}
