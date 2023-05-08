#include "../include/pinger.h"
#include "../include/errors.h"
#include "../include/host_addr.h"
#include "../include/raw_packets.h"
#include <fcntl.h>
#include <unistd.h>

class RecvEcho: public IEvent{
private:
    std::set<std::string> ips;
    msghdr msg;
    int fd;
    bool end;
public:
    RecvEcho();
    void OnRead()override; 
    void OnWrite()override; 
    void OnError()override; 
    void OnTimeout()override; 
    void OnAnyEvent()override;
    short ListeningEvents() const override; 
    void ResetEvents(int events)override;
    int GetDescriptor()const override;
    bool End() const override{return end;}
    void SetEnd(){end = true;}
    const std::set<std::string>& GetIps() const{return ips;}
    virtual ~RecvEcho(){close(fd);}
};

RecvEcho::RecvEcho() : fd(-1), end(false)
{
    raw_packets::make_raw_socket(fd, IPPROTO_ICMP);
    memset(&msg, 0, sizeof(msg)); 
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void RecvEcho::OnRead()
{
    bool end_read;
    do{
        ssize_t len = raw_packets::recv_reply(fd , &msg);
        end_read = len == -1;
        if(!end_read){
            in_addr ip;
            if(raw_packets::get_echo(reinterpret_cast<char *>(
            msg.msg_iov->iov_base),len, ip))
                ips.insert(inet_ntoa(ip));
        }
        delete[] msg.msg_iov; delete[] (char*) msg.msg_control;
    }while(!end_read);
}

int RecvEcho::GetDescriptor() const{return fd;}
void RecvEcho::OnError(){errors::SysRet("recv echo");end = true;}
void RecvEcho::OnWrite(){}
void RecvEcho::OnTimeout(){}
void RecvEcho::OnAnyEvent(){}
short RecvEcho::ListeningEvents() const {return Read + Error;}
void RecvEcho::ResetEvents(int events){}

Pinger::Pinger(Scheduler& m, const std::set<std::string>& ip_set)
    : master(m), reciver(0)
{
    it = master.GetDevStat().ip_set.begin();
    send_icmp_sd= new int[send_in_time];
}

Pinger::~Pinger()
{
    delete[] send_icmp_sd;
}

bool Pinger::Execute()
{
    bool result = false;
    if(reciver == 0){
        reciver = new RecvEcho();
        master.AddToSelector(reciver);
    } else {
        bool is_send = SendEcho();
        if(!is_send){
            UpdateDevices();
            result = true;
        }
    }
    return result;
}

bool Pinger::SendEcho()
{
    bool res = false;
    for(int i = 0; it != master.GetDevStat().ip_set.end() && i < send_in_time; it++, i++){
        if(!raw_packets::make_raw_socket(send_icmp_sd[i], IPPROTO_ICMP))
            errors::Sys("ICMP socket not created.");
        sockaddr_in dest_addr = host_addr::set_addr((*it).c_str(), AF_INET);
        raw_packets::send_echo(send_icmp_sd[i], raw_packets::get_id(), 0, 
            &dest_addr, sizeof(dest_addr));
        res = true;
        close(send_icmp_sd[i]);
    }
    return res;
}

void Pinger::UpdateDevices()
{
    std::vector<NetDevice>& temp = master.GetDevStat().devices;
    const std::set<std::string>& ip_set = static_cast<RecvEcho*>(reciver)->GetIps();
    std::set<std::string>::const_iterator ip_set_it = ip_set.begin();
    for(;ip_set_it != ip_set.end(); ip_set_it++){
        NetDevice new_dev; 
        new_dev.SetIpv4(*ip_set_it);
        temp.push_back(new_dev);
    }
    reinterpret_cast<RecvEcho*>(reciver)->SetEnd();
}

