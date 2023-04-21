#ifndef NET_DEVICE_DEF
#define NET_DEVICE_DEF

#include <unordered_map>
struct NetDeviceRep;
enum DeviceType{
    Host = 0, Router = 1, Unknown = 2
};

typedef std::unordered_map<std::string, std::string> dev_info;
class NetDevice{
public:
    const dev_info& GetInfo() const;
    bool SetIpv4(std::string ip);
    bool SetIpv6(std::string ip);
    bool SetMac(std::string ip);
    bool SetType(DeviceType type);
private:
    NetDeviceRep* dev;
};

#endif // !NET_DEVICE_DEF
