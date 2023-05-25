#ifndef NODES_MANAGER_DEF
#define NODES_MANAGER_DEF

#include <string>
#include <unordered_map> 
#include <vector>
#include <utility>

enum PortCondition{Unset, Open, Closed, Filtered};
extern const char* ports_conditions[];
struct NetNode;
typedef std::unordered_map<uint16_t, PortCondition> ports_storage;
typedef std::unordered_map<std::string, NetNode> NetMap;
typedef std::pair<std::string, std::string> NetParams; // net and mask
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


class NodesManager final{
public:
    NodesManager();
    const std::vector<std::string>& GetIpsToScan()const;
    void AddNode(const NetNode& node);
    void AddPorts(const std::string& ip, const ports_storage& new_ports, ports_storage::iterator& end_it);
    void AddGate(const char* ip);
    void SetInterface(const std::string& a_interface);
    void SetIpsToScan(const std::vector<std::string>& ips);
    void SetNetParams(const std::string& net, const std::string& mask);
    void SetOwnNode(const std::string& ip, const std::string& mac);
    const std::string& GetInterface() const;
    const NetParams& GetNetParams()const;
    const NetNode& GetOwnNode()const;
    inline bool IsChanged()const{return changed;}
    std::vector<uint16_t> GetSortedPorts(const std::string& ip);
    std::vector<std::string> GetSortedIps();
    std::vector<std::string> GetIps()const;
    std::vector<std::string> GetActiveIps()const;
    NetNode* GetNodeByIp(const std::string& ip);
    const char*const GetPortCond(const std::string& ip, uint16_t port);
    std::string GetService(uint16_t port);
    std::string GetProtocol(uint16_t port);

    inline void Updated(){changed = false;}
    inline void Change(){changed = true;}
    inline void UsrStopFullScan(){user_full_scan_stop = true;}
    inline bool IsFullSanStop(){return user_full_scan_stop;}
    inline void StoppedFullScan(){user_full_scan_stop = false;}
    inline void UsrStopPortScan(){user_port_scan_stop = true;}
    inline bool IsPortScanStop(){return user_port_scan_stop;} 
    inline void StoppedPortScan(){user_port_scan_stop = false;}
private:
    std::unordered_map<std::string, NetNode> nodes_map;
    std::unordered_map<uint16_t, std::pair<std::string, std::string>> services;
    std::vector<std::string> ips_to_scan;
    std::string interface;
    NetParams params;
    bool changed;
    bool user_full_scan_stop;
    bool user_port_scan_stop;
    void InitServices();
};

#endif // !NODES_MANAGER_DEF
