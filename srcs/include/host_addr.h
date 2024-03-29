#ifndef HOST_ADDR_DEF 
#define HOST_ADDR_DEF 

#include <netinet/if_ether.h>
#include <string>
#include <map>

namespace host_addr{
    struct interface_params{
        std::string ip;
        std::string mask;
        std::string mac;
    };
    typedef std::map<std::string, interface_params> interface_map;
    sockaddr_in set_addr(const char *ip, int family);
    std::string get_own_name();
    interface_map get_net_interfaces();
    bool is_privileged();
}

#endif
