#ifndef NODES_MANAGER_DEF
#define NODES_MANAGER_DEF
#include <string>
#include <unordered_map> 
#include <set>

enum PortCondition{Open, Closed, Filtered};
struct NetNode{
    std::string ipv4_address;
    std::string mac_address;
    std::string name;
    std::string vendor;
    std::string type;
    std::unordered_map<uint16_t, PortCondition> ports;
    NetNode() : type("Host"){}
};

typedef std::unordered_map<std::string, NetNode> NetMap;
class NodesManager{
public:
    NetMap& GetMap();
    const std::set<std::string>& GetIpSet()const;
    void AddNode(const NetNode& node);
    void SetInterface(const std::string& a_interface);
    void SetIps(const std::set<std::string>& ips);
    const std::string& GetInterface() const;
    const NetNode& GetOwnNode()const;
private:
    std::unordered_map<std::string, NetNode> nodes_map;
    std::set<std::string> ip_set;
    std::string interface;
};

#endif // !NODES_MANAGER_DEF
