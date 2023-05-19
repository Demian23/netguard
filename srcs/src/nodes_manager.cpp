#include "../include/nodes_manager.h"
#include "../include/errors.h"
#include <netdb.h>

const char* ports_conditions[] = {"Unset", "Open", "Closed", "Filtered"};

NetNode::NetNode() : type("Host"){}

void NodesManager::AddNode(const NetNode &node)
{
    auto it = nodes_map.find(node.ipv4_address);
    if(it == nodes_map.end())
        nodes_map.emplace(node.ipv4_address, node);
    else {
        it->second.is_active = node.is_active; 
        //if(!node.mac_address.empty()) it->second.mac_address = node.mac_address;
        if(!node.type.empty()) it->second.type = node.type;
        if(!node.name.empty()) it->second.name = node.name;
        //if(!node.vendor.empty()) it->second.vendor = node.vendor;
    }
    Change();
}

void NodesManager::SetInterface(const std::string &a_interface)
{
    if(interface != a_interface){
        nodes_map.clear();
        interface = a_interface;
    } 
}

void NodesManager::SetIps(const std::set<std::string> &ips)
{
    ip_set = ips;
}

NetMap& NodesManager::GetMap(){return nodes_map;}


const std::string& NodesManager::GetInterface() const
{
    return interface;
}

const NetNode& NodesManager::GetOwnNode()const
{
    for(const auto& node : nodes_map)
    {
       if(node.second.type == "Own host")
           return node.second;
    }
    errors::Quit("No own device.");
    return nodes_map.end()->second;
}

const std::set<std::string>& NodesManager::GetIpSet() const{return ip_set;}

void NodesManager::AddPorts(const std::string &ip, const ports_storage &new_ports)
{
    ports_storage& specific_ports = nodes_map[ip].ports;
    for(ports_storage::const_iterator it = new_ports.begin(); it != new_ports.end(); it++){
       if(specific_ports.find(it->first) != specific_ports.end())
           specific_ports[it->first] = it->second;
       else specific_ports.emplace(it->first, it->second);
    }
}

std::string NodesManager::GetService(uint16_t port)
{
    std::string res;
    if(services.find(port) != services.end())
        res = services[port].first;
    return res; 
}
std::string NodesManager::GetProtocol(uint16_t port)
{
    std::string res;
    if(services.find(port) != services.end())
        res = services[port].second;
    return res; 
}

void NodesManager::InitServices()
{
    struct servent* temp;
    setservent(1);
    do{
        temp = getservent();
        if(temp != 0){
            uint16_t port = ntohs(temp->s_port);
            services.insert(std::make_pair(port, std::make_pair(temp->s_name, temp->s_proto)));
        }
    }while(temp != 0);
    endservent();
}

std::vector<uint16_t> NodesManager::GetSortedPorts(const std::string& ip)
{
    std::vector<uint16_t> opened;
    std::vector<uint16_t> others;
    for(ports_storage::const_iterator it = nodes_map[ip].ports.begin(); 
        it != nodes_map[ip].ports.end(); it++){
        if(it->second == Open)
            opened.push_back(it->first);
        else others.push_back(it->first);
    }
    std::sort(opened.begin(), opened.end());
    std::sort(others.begin(), others.end());
    opened.insert(opened.end(), others.begin(), others.end());
    return opened;
}

const char*const NodesManager::GetPortCond(const std::string &ip, uint16_t port)
{
    return ports_conditions[nodes_map[ip].ports[port]];
}
