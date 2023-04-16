#include <ifaddrs.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <arpa/inet.h>
#include <string.h>

#include "host_addr.h"
#include "errors.h"

namespace host_addr{

sockaddr_in set_addr(const char *ip, int family)
{
    sockaddr_in res;
    res.sin_family = family;  
    if(!inet_aton(ip, &res.sin_addr))
        errors::Quit("invalid address");
    return res;
}

bool findownaddr(std::string& interface, ether_addr& ownmac, 
    sockaddr_in& saip, sockaddr_in& samask)
{
    ifaddrs *ifa, *temp;
    sockaddr_dl *sdl;
    static const std::string loop_interface = "lo";

    bool find_interface = interface.size() == 0, find_mac = false, 
        find_ip = false;

    if(!getifaddrs(&ifa)){
        for(temp = ifa; temp; temp = temp->ifa_next){
            if(find_interface){
                if(loop_interface == 
                    std::string(temp->ifa_name).substr(0, loop_interface.size())) 
                    continue;
                else
                    interface = temp->ifa_name;
            }
            if(interface == temp->ifa_name){
                sdl = reinterpret_cast<sockaddr_dl *>(temp->ifa_addr); 
                if(sdl->sdl_family == AF_LINK && sdl->sdl_type == IFT_ETHER
                        && sdl->sdl_alen == ETHER_ADDR_LEN){
                    memcpy(reinterpret_cast<uint8_t *>(&ownmac), 
                            reinterpret_cast<uint8_t *>(LLADDR(sdl)), 
                            sizeof(ether_addr));
                    find_mac = true;
                } else {
                    if(sdl->sdl_family == AF_INET){
                        saip.sin_addr.s_addr = ((sockaddr_in *)temp->ifa_addr)->sin_addr.s_addr;
                        saip.sin_family = AF_INET;
                        samask.sin_addr.s_addr = ((sockaddr_in *)temp->ifa_netmask)->sin_addr.s_addr;
                        find_ip = true;
                    }
                }
            }
            if(find_mac && find_ip)
                break;
        }
    } else{
        errors::Msg("Get own addressess mistake");
    }
    freeifaddrs(ifa);
    return find_mac && find_ip;
}


};


std::string host_addr::get_local_ip()
{
    ifaddrs *interface = new ifaddrs;
    if(getifaddrs(&interface) != 0)
        return std::string();
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
    return std::string(res);
}

