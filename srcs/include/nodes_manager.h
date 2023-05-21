#ifndef NODES_MANAGER_DEF
#define NODES_MANAGER_DEF
#include <string>
#include <unordered_map> 
#include <set>
#include <vector>

enum PortCondition{Unset, Open, Closed, Filtered};
extern const char* ports_conditions[];
struct NetNode;
typedef std::unordered_map<uint16_t, PortCondition> ports_storage;
typedef std::unordered_map<std::string, NetNode> NetMap;
struct NetNode{
    std::string ipv4_address;
    std::string mac_address;
    std::string name;
    std::string vendor;
    std::string type;
    ports_storage ports;
    bool is_active;
    NetNode();
};

class NodesManager{
public:
    NodesManager() : changed(false), user_full_scan_stop(false), user_port_scan_stop(false){}
    const std::set<std::string>& GetIpSet()const;
    void AddNode(const NetNode& node);
    void AddPorts(const std::string& ip, const ports_storage& new_ports, ports_storage::iterator& end_it);
    void SetInterface(const std::string& a_interface);
    void SetIpSet(const std::set<std::string>& ips);
    const std::string& GetInterface() const;
    const NetNode& GetOwnNode()const;
    void InitServices();
    bool IsChanged()const{return changed;}
    void Updated(){changed = false;}
    void Change(){changed = true;}
    std::vector<uint16_t> GetSortedPorts(const std::string& ip);
    std::vector<std::string> GetSortedIps();
    std::vector<std::string> GetIps()const;
    std::vector<std::string> GetActiveIps()const;
    NetNode* GetNodeByIp(const std::string& ip);
    const char*const GetPortCond(const std::string& ip, uint16_t port);
    std::string GetService(uint16_t port);
    std::string GetProtocol(uint16_t port);
    void SetAllNodesInactive();
    void AlarmInactiveNodes();
    void UsrStopFullScan(){user_full_scan_stop = true;}
    inline bool IsFullSanStop(){return user_full_scan_stop;}
    inline void StoppedFullScan(){user_full_scan_stop = false;}
    void UsrStopPortScan(){user_port_scan_stop = true;}
    inline bool IsPortScanStop(){return user_port_scan_stop;} 
    inline void StoppedPortScan(){user_port_scan_stop = false;}
private:
    std::unordered_map<std::string, NetNode> nodes_map;
    std::unordered_map<uint16_t, std::pair<std::string, std::string>> services;
    std::set<std::string> ip_set;
    std::string interface;
    bool changed;
    bool user_full_scan_stop;
    bool user_port_scan_stop;
};

#endif // !NODES_MANAGER_DEF
