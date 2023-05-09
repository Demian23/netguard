#include "../include/mac.h"
#include "../include/errors.h"
#include <fstream>
#include <cstring>

namespace MAC{
enum{first_part_size = 8};    
const char *vendors_file = "./build/resources/mac-vendors-export.csv";

std::string mac_to_string(const ether_addr &mac)
{
    return ether_ntoa(&mac);
}

std::string get_vendor(const ether_addr &mac)
{
    char octets[first_part_size + 1] = {};
    sprintf(octets, "%02X:%02X:%02X", mac.octet[0], 
        mac.octet[1], mac.octet[2]); 
    std::ifstream in;
    std::string res;
    in.open(vendors_file);
    while(std::getline(in, res))
        if(res.substr(0, first_part_size) == octets)
            break;
    return res;
}

};
