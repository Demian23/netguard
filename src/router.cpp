#include "../include/router.h"
#include "../include/raw_packets.h"
#include <unistd.h>

bool FindGate::Execute()
{
    int fd, id = -1;
    msghdr msg; in_addr res;
    bool flag;
    raw_packets::send_ttl_1(fd, id);
    ssize_t len = raw_packets::recv_reply(fd, &msg);
    flag = raw_packets::get_exceed_node(fd, (char*)msg.msg_iov->iov_base, len, id, res);
    delete[] msg.msg_iov; delete[] (char*)msg.msg_control;
    close(fd);
    if(flag){
        std::string res_str = inet_ntoa(res);
        std::vector<NetDevice>& temp = master.GetDevStat().devices;
        for(int i = 0; i < temp.size(); i++)
            if(temp[i].GetIp() == res_str){
                temp[i].SetType(Gateway);
                break;
            }
    }
    return flag;
}

