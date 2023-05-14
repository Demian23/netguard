#include <algorithm>
#include <gtest/gtest.h>
#include <iostream>
#include <sys/socket.h>
#include "../../srcs/include/raw_packets.h"
#include "../../srcs/include/host_addr.h"
#include "../../srcs/include/scheduler.h"
#include "../../srcs/include/port_scanner.h"
#include "../../srcs/include/errors.h"
#include "../../srcs/include/ip.h"

TEST(HostAddrSuit, GetNetInterfaces){
    auto interfaces = host_addr::get_net_interfaces();    
    ASSERT_TRUE(interfaces.size() != 0);
    for(host_addr::interface_map::iterator it = interfaces.begin(); it != interfaces.end();
        it++)
        std::cout << "Name: "<<it->first << "\nNet: "<< it->second.net << 
            "\nMask: "<< it->second.mask << "\nMac: " << it->second.mac <<"\n\n";
}

TEST(PortScannerSuit, RecieveSendAnswer){
   int fd; 
   char buffer[1024]={};
   sockaddr_in t;
   sockaddr_in src = {.sin_family = AF_INET, .sin_addr = IP::str_to_ip("192.168.1.218")};
   sockaddr_in dest = {.sin_family = AF_INET, .sin_addr = IP::str_to_ip("192.168.1.1")};
//   src.sin_port = htons(50345);
   raw_packets::make_raw_socket(fd, IPPROTO_TCP);
//   if(-1==bind(fd, (sockaddr*)&src, sizeof(src)))
//       errors::Sys("");
   raw_packets::send_tcp_flag(fd, src, &dest, 50345, 12, TH_SYN);
   ssize_t res_len = raw_packets::recieve_packet(fd, buffer, 1024, &t);
   ASSERT_NE(res_len, -1);
   raw_packets::get_syn_answer(buffer, res_len, &src);
   std::cout << inet_ntoa(src.sin_addr) << " " << src.sin_port << std::endl;
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
