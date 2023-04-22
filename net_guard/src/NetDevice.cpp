

#include "NetDevice.h"
#include "../../net/src/errors.h"

struct NetDeviceRep{
    int ref_counter;
    in_addr* ipv4;
    in6_addr* ipv6;
    ether_addr* mac;
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
    NetDeviceRep() : ipv4(0), ipv6(0), mac(0), type(Unknown), changed(false)
    {
        ref_counter = 1;
        cached_info = {
            {"Ip", ""},
            {"Ipv6", ""},
            {"MAC", ""}, 
            {"Type", ""}, 
            {"Vendor", ""}
        }; 
    }
    ~NetDeviceRep(){if(ipv4) delete ipv4; if(ipv6) delete ipv6; if(mac) delete mac;}
private: 
    NetDeviceRep(const NetDeviceRep&);
    NetDeviceRep& operator=(const NetDeviceRep&);
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
    if(ipv4)
        return AddrToString(AF_INET, ipv4);
    else return "";
}

std::string NetDeviceRep::Ipv6ToString()
{
    if(ipv6)
        return AddrToString(AF_INET6, ipv6);
    else return "";
}

std::string NetDeviceRep::MacToString()
{
    enum{max_mac_len = 18};
    char mac_str[max_mac_len] = {};
    sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",mac->octet[0], 
        mac->octet[1], mac->octet[2], mac->octet[3], mac->octet[4], mac->octet[5]);
    return mac_str;
}

std::string NetDeviceRep::TypeToString()
{
    static const char*const types_str[] = {"Host", "Router", "Unknown"};
    return types_str[type];
}

std::string NetDeviceRep::VendorToString(){return vendor;}

bool NetDevice::SetIpv4(in_addr ip)
{
    //TODO: check on valid
    if(!dev->ipv4)
        dev->ipv4 = new in_addr;
   *dev->ipv4 = ip; 
   return true;
}

NetDevice::NetDevice()
{
    dev = new NetDeviceRep();
}

NetDevice::NetDevice(const NetDevice& c)
{
    c.dev->ref_counter++;
    dev = c.dev;
}

NetDevice& NetDevice::operator=(const NetDevice& c)
{
    c.dev->ref_counter++;
    if(--dev->ref_counter == 0) delete dev;
    dev = c.dev;
    return *this;
}

NetDevice::~NetDevice()
{
    if(--dev->ref_counter == 0)
        delete dev;
}

bool NetDevice::SetIpv6(in6_addr ip)
{
    if(!dev->ipv6){
        dev->ipv6 = new in6_addr;    
    }
    *dev->ipv6 = ip;
    return dev->changed = true;
}

bool NetDevice::SetMac(ether_addr mac)
{
    if(!dev->mac){
        dev->mac = new ether_addr;
    }
    *dev->mac = mac;
    return dev->changed = true;
}

bool NetDevice::SetIpv4(const std::string& ip)
{
    if(!dev->ipv4)
        dev->ipv4 = new in_addr;
    const char* ip_c = ip.c_str(); 
    inet_aton(ip_c, dev->ipv4); 
    return dev->changed = true;
}

std::string NetDevice::GetIp() const
{
    if(dev->ipv4)
        return inet_ntoa(*dev->ipv4);
    else return "";
}

bool NetDevice::HasMac() const
{
    return dev->mac ? true : false;
}
