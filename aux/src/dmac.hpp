#ifndef DMAC_DEF
#define DMAC_DEF
#include <net/ethernet.h>
namespace DMAC{
    struct mac_vendor{
        ether_addr eth;
        char *vendor;
        bool find;
        ~mac_vendor(){delete[] vendor;}
    };
    mac_vendor* vendors_arr(const ether_addr *arr, int len);
}; 
#endif 
