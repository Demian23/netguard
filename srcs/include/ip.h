#ifndef IP_DEF
#define IP_DEF

#include <stdint.h>
#include <arpa/inet.h>
#include <vector>
#include <string>

namespace IP{
    enum{max_mask_prefix = 32};
    enum DevType{endpoint, customer_premise};
    uint32_t ipv4_to_number(const std::string& ip);
    std::string number_to_ipv4(const uint32_t number);

    std::string ipv4_net(const std::string& some_ip, const std::string& mask);
    std::string ipv4_net(const std::string& some_ip, const short mask_prefix);
    std::vector<std::string> all_ipv4_from_range(const std::string& first, const std::string& last);

    uint32_t ip_amount(const short mask_prefix);
    uint32_t mask_number(const short mask_prefix);
    short mask_prefix(const std::string& mask);

    std::string get_name(sockaddr_in* addr);
    in_addr str_to_ip(const char* ip_str);
    int itoa(int n, char s[]);
    std::string last_ip(const std::string& net, const std::string& mask);
    std::string first_ip(const std::string& net);
    bool check_ip_range(const std::string& net, const std::string& mask, const std::string& first,
        const std::string& last);
    bool is_ip_in_net(const std::string& net, const std::string& mask, 
        const std::string& ip);
    bool is_valid_ip_string(const std::string& ip_string);
};
#endif
