#ifndef NET_DEVICE_DEF
#define NET_DEVICE_DEF

#include <map>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <string>
#include <set>
#include <vector>

struct NetDeviceRep;
enum DeviceType{
    Host = 0, Router = 1, Unknown = 2, OwnHost = 3, Gateway = 4
};
typedef std::map<std::string, std::string> dev_info;

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
    void GetIp(sockaddr_in& addr) const;
    void GetMac(ether_addr& mac) const;
    bool HasMac() const;
    DeviceType GetType() const;
    ~NetDevice();
    NetDevice(const NetDevice& c);
    NetDevice& operator=(const NetDevice& c);
private:
    NetDeviceRep* dev;
};
void print(const NetDevice& dev);
struct Info{
    std::set<std::string> ip_set;
    std::string interface;
    std::vector<NetDevice>& devices;
};

#endif // !NET_DEVICE_DEF
