#include <unistd.h>
#include "../include/router.h"
#include "../include/raw_packets.h"
#include "../include/ip.h"

bool FindGate::Execute()
{
    int fd, id = -1;
    in_addr res; sockaddr_in from;
    char buffer[1024] = {};
    short ttl = 1;
    bool flag;
    const NetParams& p = master.manager.GetNetParams();
    do{
        raw_packets::send_ttl(fd, id, ttl);
        ssize_t len = raw_packets::recieve_packet(fd, buffer, 1024, &from);
        flag = raw_packets::get_exceed_node(fd, buffer, len, id, res) && 
            IP::is_ip_in_net(p.first, p.second, inet_ntoa(res));
        if(flag){
            master.manager.AddGate(inet_ntoa(res));
            ttl++;
            res.s_addr = 0;
        }
        close(fd);
    } while(flag);
    return true;
}

