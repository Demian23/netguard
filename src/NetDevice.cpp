#include "../include/NetDevice.h"
#include "../include/errors.h"
#include "../include/mac.h"
#include <netdb.h>

struct NetDeviceRep{
    int ref_counter;
    in_addr* ipv4;
    in6_addr* ipv6;
    ether_addr* mac;
    DeviceType type;    
    std::string vendor;
    std::string name;
    bool changed;
    dev_info cached_info;
    void FillInfo();
    std::string AddrToString(int family, const void* addr);
    std::string IpToString();
    std::string Ipv6ToString();
    std::string MacToString();
    std::string TypeToString();
    std::string VendorToString();
    std::string NameToString();
    NetDeviceRep() : ipv4(0), ipv6(0), mac(0), type(Unknown), changed(false)
    {
        type = Host;
        ref_counter = 1;
        cached_info = {
            {"Type", ""}, 
            {"Name", ""}, 
            {"Ip", ""},
            {"Ipv6", ""},
            {"MAC", ""}, 
            {"Vendor", ""}
        }; 
    }
    ~NetDeviceRep(){if(ipv4) delete ipv4; if(ipv6) delete ipv6; if(mac) delete mac;}
private: 
    NetDeviceRep(const NetDeviceRep&);
    NetDeviceRep& operator=(const NetDeviceRep&);
};

typedef std::string (NetDeviceRep::*update_delegate)(void);
static std::map<std::string, update_delegate> update_key_value= {
    {"Type", &NetDeviceRep::TypeToString},
    {"Name", &NetDeviceRep::NameToString},
    {"Ip", &NetDeviceRep::IpToString},
    {"Ipv6", &NetDeviceRep::Ipv6ToString},
    {"MAC", &NetDeviceRep::MacToString},
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
    static const char*const types_str[] = {"Host", "Router", "Unknown", "Own host", "Gateway"};
    return types_str[type];
}

std::string NetDeviceRep::VendorToString(){return vendor;}
std::string NetDeviceRep::NameToString(){return name;}

bool NetDevice::SetIpv4(in_addr ip)
{
    //TODO: check on valid
    if(!dev->ipv4)
        dev->ipv4 = new in_addr;
   *dev->ipv4 = ip; 
   sockaddr_in ip_addr = {.sin_family = AF_INET, .sin_addr = *dev->ipv4};
    char hostname[NI_MAXHOST] = {};
    getnameinfo(reinterpret_cast<const sockaddr *>(&ip_addr), sizeof(sockaddr_in), 
            hostname, NI_MAXHOST, 0, 0, 0);
    dev->name = hostname;
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
    MAC::mac_vendor v = MAC::vendor(mac);     
    if(v.find)
        dev->vendor = std::string(v.info[0]) + "\nPrivate: " + v.info[1] + ", Block type: " 
            + v.info[2] + ", Last update: " + v.info[3];
    return dev->changed = true;
}

bool NetDevice::SetIpv4(const std::string& ip)
{
    in_addr tmp;
    inet_aton(ip.c_str(), &tmp); 
    return SetIpv4(tmp);
}

std::string NetDevice::GetIp() const
{
    if(dev->ipv4)
        return inet_ntoa(*dev->ipv4);
    else return "";
}

void NetDevice::GetIp(sockaddr_in& addr) const
{
    if(dev->ipv4){
        addr.sin_family = AF_INET;
        addr.sin_addr = *dev->ipv4;
    }
}

void NetDevice::GetMac(ether_addr &mac) const
{
    if(dev->mac)
        mac = *dev->mac;
}

bool NetDevice::HasMac() const
{
    return dev->mac ? true : false;
}

DeviceType NetDevice::GetType() const
{
    return dev->type;
}

bool NetDevice::SetType(DeviceType type)
{
    dev->type = type;
    return dev->changed = true;
}
void print(const NetDevice& dev)
{
    const dev_info& info = dev.GetInfo();
    dev_info::const_iterator it = info.begin();
    for(;it != info.end(); it++){
        if(!it->second.empty())
            printf("%s: %s\n", it->first.c_str(), it->second.c_str());
    }
    putchar('\n');
}
