#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <string>


#include "NetDevice.h"
#include "../../net/src/errors.h"

struct NetDeviceRep{
    in_addr ipv4;
    sockaddr_in6 ipv6;
    ether_addr mac;
    DeviceType type;    
    std::string vendor;
    bool changed;
    dev_info cached_info;
    void FillInfo();
    std::string AddrToString(int family, const void* addr);
    std::string IpToString();
    std::string Ipv6ToString();
    std::string MacToString();
    std::string TypeToString();
    std::string VendorToString();
    NetDeviceRep() : changed(false)
    {
        cached_info = {
            {"Ip", ""},
            {"Ipv6", ""},
            {"MAC", ""}, 
            {"Type", ""}, 
            {"Vendor", ""}
        }; 
    }
};

typedef std::string (NetDeviceRep::*update_delegate)(void);
static std::unordered_map<std::string, update_delegate> update_key_value= {
    {"Ip", &NetDeviceRep::IpToString},
    {"Ipv6", &NetDeviceRep::Ipv6ToString},
    {"MAC", &NetDeviceRep::MacToString},
    {"Type", &NetDeviceRep::TypeToString},
    {"Vendor", &NetDeviceRep::VendorToString},
};

void NetDeviceRep::FillInfo()
{
    for(dev_info::iterator it = cached_info.begin(); it != cached_info.end();
        it++){
        it->second = (this->*update_key_value.find(it->first)->second)();
    }
}

const dev_info& NetDevice::GetInfo() const 
{
    if(dev->changed){
        dev->FillInfo();
    } 
    return dev->cached_info;
}

std::string NetDeviceRep::AddrToString(int family, const void *addr)
{
    enum{addr_str_size = 0xFF};
    char addr_str[addr_str_size] = {};
    std::string res;
    const char* temp = inet_ntop(family, addr, addr_str, addr_str_size);
    if(temp != 0){
        res = temp;
    } else {
        errors::SysRet("AddrToString"); 
    }
    return res;
}

std::string NetDeviceRep::IpToString()
{
    return AddrToString(AF_INET, &ipv4);
}

std::string NetDeviceRep::Ipv6ToString()
{
    return AddrToString(AF_INET6, &ipv6.sin6_addr);
}

std::string NetDeviceRep::MacToString()
{
    enum{max_mac_len = 18};
    char mac_str[max_mac_len] = {};
    sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",mac.octet[0], 
        mac.octet[1], mac.octet[2], mac.octet[3], mac.octet[4], mac.octet[5]);
    return mac_str;
}

std::string NetDeviceRep::TypeToString()
{
    static const char*const types_str[] = {"Host", "Router", "Unknown"};
    return types_str[type];
}

std::string NetDeviceRep::VendorToString(){return vendor;}
