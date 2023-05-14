#include "../include/pinger.h"
#include "../include/errors.h"
#include "../include/host_addr.h"
#include "../include/raw_packets.h"
#include <iterator>

class RecvEcho: public IEvent{
private:
    enum{buffer_size = 1024};
    std::set<std::string> ips;
    char* buffer;
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
    virtual ~RecvEcho(){close(fd); delete[] buffer;}
};

RecvEcho::RecvEcho() : fd(-1), end(false)
{
    raw_packets::make_raw_socket(fd, IPPROTO_ICMP);
    buffer = new char[buffer_size];
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void RecvEcho::OnRead()
{
    bool end_read;
    sockaddr_in from;
    do{
        ssize_t len = raw_packets::recieve_packet(fd, buffer, buffer_size, &from);
        end_read = len == -1;
        if(!end_read){
            in_addr ip;
            if(raw_packets::get_echo(buffer, len, ip))
                ips.insert(inet_ntoa(ip));
        }
    }while(!end_read);
}

int RecvEcho::GetDescriptor() const{return fd;}
void RecvEcho::OnError(){errors::SysRet("recv echo");end = true;}
void RecvEcho::OnWrite(){}
void RecvEcho::OnTimeout(){}
void RecvEcho::OnAnyEvent(){}
short RecvEcho::ListeningEvents() const {return Read + Error;}
void RecvEcho::ResetEvents(int events){}

Pinger::Pinger(Scheduler& m, Statictic* stat)
    : master(m), statistic(stat), ips_set(master.manager.GetIpSet()), reciver(0)
{
    it = ips_set.begin();
    send_icmp_sd = new int[send_in_time];
}

Pinger::~Pinger()
{
    delete statistic;
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
        statistic->RecordStatistic(this);
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
    for(int i = 0; it != ips_set.end() && i < send_in_time; it++, i++){
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
    const std::set<std::string>& ip_set = static_cast<RecvEcho*>(reciver)->GetIps();
    std::set<std::string>::const_iterator ip_set_it = ip_set.begin();
    for(;ip_set_it != ip_set.end(); ip_set_it++){
        NetNode new_node; 
        new_node.is_active = true;
        new_node.ipv4_address = *ip_set_it;
        sockaddr_in temp;temp.sin_family = AF_INET; 
        temp.sin_addr = IP::str_to_ip(new_node.ipv4_address.c_str());
        new_node.name = IP::get_name(&temp);
        master.manager.AddNode(new_node);
    }
    reinterpret_cast<RecvEcho*>(reciver)->SetEnd();
}

int Pinger::GetCurrentCount() const
{
    return std::distance(ips_set.begin(), it);
}
