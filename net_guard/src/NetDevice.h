#ifndef NET_DEVICE_DEF
#define NET_DEVICE_DEF

#include <unordered_map>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <string>
struct NetDeviceRep;
enum DeviceType{
    Host = 0, Router = 1, Unknown = 2
};

typedef std::unordered_map<std::string, std::string> dev_info;

class NetDevice{
public:
    NetDevice();
    const dev_info& GetInfo() const;
    bool SetIpv4(in_addr ip);
    bool SetIpv6(in6_addr ip);
    bool SetMac(ether_addr mac);
    bool SetIpv4(const std::string& ip);
    bool SetIpv6(const std::string& ip);
    bool SetMac(const std::string& mac);
    bool SetType(DeviceType type);
    std::string GetIp() const;
    bool HasMac() const;
    ~NetDevice();
    NetDevice(const NetDevice& c);
    NetDevice& operator=(const NetDevice& c);
private:
    NetDeviceRep* dev;
};

#endif // !NET_DEVICE_DEF
