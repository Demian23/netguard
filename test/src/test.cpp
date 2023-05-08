#include <gtest/gtest.h>
#include "../../include/raw_packets.h"
#include "../../include/host_addr.h"

TEST(PortScanTestSuit, SendSyn){
    int sd;
    ASSERT_EQ(raw_packets::make_raw_socket(sd, IPPROTO_TCP), true);
    sockaddr_in src, dest;
    ASSERT_EQ(1, inet_pton(AF_INET, "192.168.1.1", &dest.sin_addr));
    ASSERT_EQ(1, inet_pton(AF_INET, "192.168.1.15", &src.sin_addr));
    src.sin_port = htons(40096);
    ASSERT_EQ(raw_packets::send_syn(sd, src, &dest, 40096, 21), true);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
