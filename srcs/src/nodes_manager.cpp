#include "../include/nodes_manager.h"
#include "../include/errors.h"

void NodesManager::AddNode(const NetNode &node)
{
    nodes_map[node.ipv4_address] = node;
}

void NodesManager::SetInterface(const std::string &a_interface)
{
    interface = a_interface;
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
