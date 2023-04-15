#ifndef HOST_ADDR_DEF 
#define HOST_ADDR_DEF 

#include <netinet/in.h>
#include <netinet/if_ether.h>
#include "String.h"

namespace host_addr{
    sockaddr_in set_addr(const char *ip, int family);
    String get_local_ip();
    bool findownaddr(String& interface, ether_addr& ownmac, 
        sockaddr_in& saip, sockaddr_in& samask);
}

#endif
