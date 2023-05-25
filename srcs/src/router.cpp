#include "../include/router.h"
#include "../include/raw_packets.h"
#include <unistd.h>

bool FindGate::Execute()
{
    int fd, id = -1;
    in_addr res; sockaddr_in from;
    char buffer[1024] = {};
    bool flag = true;
    raw_packets::send_ttl_1(fd, id);
    ssize_t len = raw_packets::recieve_packet(fd, buffer, 1024, &from);
    flag = raw_packets::get_exceed_node(fd, buffer, len, id, res);
    close(fd);
    if(flag)
        master.manager.GetNodeByIp(inet_ntoa(res))->type = "Gateway";
    return flag;
}

bool FindRouters::Execute()
{
    int sd = -1, id = -1;
    raw_packets::make_raw_socket(sd, IPPROTO_ICMP);
    raw_packets::send_irc(sd, id);
    close(sd);
    return true;
}
