#ifndef NETWORK_DEFAUTL_DEF
#define NETWORK_DEFAUTL_DEF

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/if_ether.h>


namespace DNET{
    struct sockaddr_in set_addr(const char *ip, int family);
    char* get_local_ip();
    bool findownaddr(char *&interface, struct ether_addr *ownmac, 
        struct sockaddr_in *saip, struct sockaddr_in *samask);
};

#endif
