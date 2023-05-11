#include <algorithm>
#include <gtest/gtest.h>
#include <iostream>
#include "../../include/raw_packets.h"
#include "../../include/host_addr.h"

TEST(HostAddrSuit, GetNetInterfaces){
    auto interfaces = host_addr::get_net_interfaces();    
    ASSERT_TRUE(interfaces.size() != 0);
    for(host_addr::interface_map::iterator it = interfaces.begin(); it != interfaces.end();
        it++)
        std::cout << "Name: "<<it->first << "\nNet: "<< it->second.net << 
            "\nMask: "<< it->second.mask << "\nMac: " << it->second.mac <<"\n\n";
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
