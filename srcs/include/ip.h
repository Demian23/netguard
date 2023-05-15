#ifndef IP_DEF
#define IP_DEF

#include <stdint.h>
#include <arpa/inet.h>
#include <set>
#include <string>

namespace IP{
    enum{max_mask_prefix = 32};
    enum DevType{endpoint, customer_premise};
    uint32_t ipv4_to_number(const std::string& ip);
    std::string number_to_ipv4(const uint32_t number);

    std::string ipv4_net(const std::string& some_ip, const std::string& mask);
    std::string ipv4_net(const std::string& some_ip, const short mask_prefix);
    std::set<std::string> all_net_ipv4(const std::string& net, uint32_t start, uint32_t interval_size);
    std::set<std::string> all_ipv4_from_range(const std::string& first, const std::string& last);

    uint32_t ip_amount(const short mask_prefix);
    uint32_t mask_number(const short mask_prefix);
    short mask_prefix(const std::string& mask);

    DevType devtype_from_vendor(const char *vendor); 

    std::string get_name(sockaddr_in* addr);
    in_addr str_to_ip(const char* ip_str);
    int itoa(int n, char s[]);
    std::string last_ip(const std::string& net, const std::string& mask);
    std::string first_ip(const std::string& net);
    std::set<std::string> all_ipv4_from_range(const std::string &first, const std::string &last);
};
#endif
