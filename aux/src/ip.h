#ifndef IP_DEF
#define IP_DEF

#include <stdint.h>
#include <arpa/inet.h>

namespace IP{
    enum{max_mask_prefix = 32};
    enum DevType{endpoint, customer_premise};
    uint32_t ipv4_to_number(const char *ip);
    char *number_to_ipv4(const uint32_t number);

    char *ipv4_net(const char *some_ip, const char *mask);
    char *ipv4_net(const char *some_ip, const short mask_prefix);
    char** all_net_ipv4(const char *net, uint32_t start, uint32_t interval_size);

    uint32_t ip_amount(const short mask_prefix);
    uint32_t mask_number(const short mask_prefix);
    short mask_prefix(const char *mask);

    DevType devtype_from_vendor(const char *vendor); 
};
#endif
