#include <net/if_dl.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if_types.h>

#include "../include/host_addr.h"
#include "../include/errors.h"
#include "../include/mac.h"
#include "../include/ip.h"

namespace host_addr{

std::string get_own_name()
{
    char buffer[1024]={};
    gethostname(buffer, 1024);
    return buffer;
}

sockaddr_in set_addr(const char *ip, int family)
{
    sockaddr_in res;
    res.sin_family = family;  
    if(!inet_aton(ip, &res.sin_addr))
        errors::SysRet("invalid address %s", ip);
    return res;
}

interface_map get_net_interfaces()
{
    ifaddrs *ifa, *temp;
    sockaddr_dl *sdl;
    interface_map result;
    if(!getifaddrs(&ifa)){
        for(temp = ifa; temp; temp = temp->ifa_next){
            sdl = reinterpret_cast<sockaddr_dl *>(temp->ifa_addr); 
            if(sdl->sdl_family == AF_LINK && sdl->sdl_type == IFT_ETHER
                    && sdl->sdl_alen == ETHER_ADDR_LEN){
                ether_addr mac;
                memcpy(reinterpret_cast<uint8_t *>(&mac), 
                        reinterpret_cast<uint8_t *>(LLADDR(sdl)), 
                        sizeof(ether_addr));
                result[temp->ifa_name].mac = MAC::mac_to_string(mac);
            } else if(sdl->sdl_family == AF_INET){
                in_addr ip, mask; 
                ip = reinterpret_cast<sockaddr_in*>(temp->ifa_addr)->sin_addr;
                mask =reinterpret_cast<sockaddr_in*>(temp->ifa_netmask)->sin_addr; 
                result[temp->ifa_name].ip = inet_ntoa(ip);
                result[temp->ifa_name].mask = inet_ntoa(mask);
            }
        }
        freeifaddrs(ifa);
    } else {
        errors::Sys("Unsuccessful interface retrieve");
    }   
    for(host_addr::interface_map::iterator it = result.begin(); it != result.end();){
        if(it->second.mac.empty() || it->second.ip.empty() || it->second.mask.empty())
            it = result.erase(it);
        else it++;
    }
    return result;
}

};
