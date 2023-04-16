#ifndef HOST_ADDR_DEF 
#define HOST_ADDR_DEF 

#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <string>

namespace host_addr{
    sockaddr_in set_addr(const char *ip, int family);
    std::string get_local_ip();
    bool findownaddr(std::string& interface, ether_addr& ownmac, 
        sockaddr_in& saip, sockaddr_in& samask);
}

#endif
