#ifndef MAC_DEF
#define MAC_DEF

#include <string>
#include <net/ethernet.h>
namespace MAC{
    std::string mac_to_string(const ether_addr& mac); 
    std::string get_vendor(const ether_addr& mac);
}; 

#endif
