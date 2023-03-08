#ifndef DMAC_DEF
#define DMAC_DEF
#include <net/ethernet.h>
namespace DMAC{
    struct mac_vendor{
        enum{info_size = 4};
        ether_addr eth;
        char *octets;
        //in order vendor, private, block type, last update
        char **info;
        bool find;
        ~mac_vendor(){delete[] octets; if(find)for(int i = 0; i < info_size; i++)delete[] info[i];delete[] info;}
    };
    mac_vendor* vendors_arr(const ether_addr *arr, int len);
}; 
#endif 
