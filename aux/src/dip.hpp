#ifndef DIP_DEF
#define DIP_DEF
#include <stdint.h>
namespace DIP{
    uint32_t ipv4_to_number(const char *ip);
    char *number_to_ipv4(const uint32_t number);

    char *ipv4_net(const char *some_ip, const char *mask);
    char *ipv4_net(const char *some_ip, const short mask_prefix);
    char** all_net_ipv4(const char *net, uint32_t start, uint32_t interval_size);

    uint32_t ip_amount(const short mask_prefix);
    uint32_t mask_number(const short mask_prefix);
    short mask_prefix(const char *mask);
};
#endif
