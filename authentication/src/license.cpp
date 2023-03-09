#include "license.hpp"
#include "../../aux/src/dnet.hpp"
#include "../../aux/src/dneterr.hpp"
#include <cstdio>
#include <sys/types.h>
#include <sys/sysctl.h>

namespace License{
    enum{dev_arr_size = 1};
    struct Device{
        enum{mac_symbols_amount = 17};
        char **valid_macs; 
        int valid_macs_size;
        const char *dev_model;
        const char *ostype;
        int32_t cpu_model;
        int64_t cpu_frequency;
        ~Device(){for(int i = 0; i < valid_macs_size; i++)delete[] valid_macs[i]; delete[] valid_macs;}
    };
    Device dev_arr[] = {{0, 0, "iMac17,1", "Darwin", 94, 3312000000}};
    void init_macs();
    bool is_valid_mac(const char *interface, const Device &dev);
    bool is_valid_machine_params(const Device &dev);
};

bool License::is_valid_device(const char *interface)
{
    init_macs();
    for(int i = 0; i < dev_arr_size; i++) 
        if(is_valid_mac(interface, dev_arr[i]) && is_valid_machine_params(dev_arr[i]))
            return true;
    return false;
}

void License::init_macs()
{
    const char *valid_mac[] = {"a8:60:b6:16:6a:81", "28:f0:76:6c:0b:52"};
    dev_arr[0].valid_macs_size = 2;
    dev_arr[0].valid_macs = new char *[dev_arr[0].valid_macs_size];
    for(int i = 0; i < dev_arr[0].valid_macs_size; i++){
        dev_arr[0].valid_macs[i] = new char [Device::mac_symbols_amount]; 
        strncpy(dev_arr[0].valid_macs[i], valid_mac[i], Device::mac_symbols_amount);
    }
}

bool License::is_valid_machine_params(const Device &dev)
{
    size_t size = 256;
    int32_t model;
    int64_t frequency;
    char temp[256] = {};
    
    if(sysctlbyname("hw.model", temp, &size, 0, 0) != 0){
        DERR::Sys("sysctlbyname, License::is_valid_machine_params");
    }
    if(strcmp(dev.dev_model, temp))
        return false;
    memset(temp, 0, size);
    size = 256;
    if(sysctlbyname("kern.ostype", temp, &size, 0, 0) != 0){
        DERR::Sys("sysctlbyname, License::is_valid_machine_params");
    }
    if(strcmp(dev.ostype, temp))
        return false;
    memset(temp, 0, size);
    size = sizeof(model);
    
    if(sysctlbyname("machdep.cpu.model", &model, &size, 0, 0) != 0){
        DERR::Sys("sysctlbyname, License::is_valid_machine_params");
    }
    if(model != dev.cpu_model)
        return false;

    size = sizeof(frequency);
    if(sysctlbyname("machdep.tsc.frequency", &frequency, &size, 0, 0) != 0){
        DERR::Sys("sysctlbyname, License::is_valid_machine_params");
    }
    if(frequency != dev.cpu_frequency)
        return false;
    return true;
}

bool License::is_valid_mac(const char *interface, const Device &dev)
{
    ether_addr mac;
    sockaddr_in ip, mask;
    char temp[18] = {};
    if(!DNET::findownaddr(interface, &mac, &ip, &mask))
       return false; 
    sprintf(temp, "%02x:%02x:%02x:%02x:%02x:%02x", mac.octet[0], mac.octet[1],
        mac.octet[2],mac.octet[3],mac.octet[4],mac.octet[5]);
    for(int i = 0; i < dev.valid_macs_size; i++)
        if(!strncmp(dev.valid_macs[i], temp, dev.mac_symbols_amount))
            return true;
    return false;
    
}
