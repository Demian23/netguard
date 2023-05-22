#include <iterator>
#include <set>

#include "../include/pinger.h"
#include "../include/errors.h"
#include "../include/host_addr.h"
#include "../include/raw_packets.h"
#include "../include/ip.h"

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
    void OnError()override; 
    short ListeningEvents() const override; 
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
void RecvEcho::OnError(){errors::SysRet("ERROR: error while receiving echo reply");end = true;}
short RecvEcho::ListeningEvents() const {return Read + Error;}

Pinger::Pinger(Scheduler& m)
    : master(m), ips(m.manager.GetIpsToScan()), ip_pos(0)
{
    raw_packets::make_raw_socket(send_sd, IPPROTO_ICMP);
    reciver = new RecvEcho();
}

bool Pinger::UpdateDevices()
{
    const std::set<std::string>& ip_set = reciver->GetIps();
    std::set<std::string>::const_iterator ip_set_it = ip_set.begin();
    for(;ip_set_it != ip_set.end(); ip_set_it++){
        NetNode* temp = master.manager.GetNodeByIp(ip_set_it->c_str());
        if(!temp){
            NetNode new_node; 
            new_node.is_active = true;
            new_node.ipv4_address = *ip_set_it;
            sockaddr_in temp;temp.sin_family = AF_INET; 
            temp.sin_addr = IP::str_to_ip(new_node.ipv4_address.c_str());
            new_node.name = IP::get_name(&temp);
            master.manager.AddNode(new_node);
        }
    }
    reciver->SetEnd();
    return true;
}

Pinger::~Pinger()
{
    close(send_sd);
}

bool Pinger::SendOneEcho()
{
    sockaddr_in dest_addr = host_addr::set_addr((ips[ip_pos]).c_str(), AF_INET);
    raw_packets::send_echo(send_sd, raw_packets::get_id(), 0, 
        &dest_addr, sizeof(dest_addr));
    return true;
}

UsrPinger::UsrPinger(Scheduler& m, Statistic* stat)
    : Pinger(m), statistic(stat)
{
    send_in_time = ips.size() / 32 + 1;
    if(send_in_time > 0xFF)
        send_in_time = 0xFF;
}

bool UsrPinger::Execute()
{
    bool result = false;
    if(master.manager.IsFullSanStop()){
        result = UpdateDevices(); 
        master.manager.StoppedFullScan();
    } else {
        if(ip_pos == 0)
            master.AddToSelector(reciver);
        bool is_send;
        for(int i = 0; ip_pos < ips.size() && i < send_in_time; ip_pos++, i++){
            is_send = SendOneEcho();
        }
        statistic->RecordStatistic(this);
        if(!is_send){
            result = UpdateDevices();
        }
    }
    return result;
}

UsrPinger::~UsrPinger(){delete statistic;}

bool InternalPinger::Execute()
{
    bool result = false;
    if(ip_pos == 0)
        master.AddToSelector(reciver);
    if(ip_pos < ips.size()){
        SendOneEcho();
        ip_pos++;
    } else {
        result = UpdateDevices();
    }
    return result;
}

bool AvailabilityPinger::UpdateDevices()
{
    std::set<std::string> recived_set = reciver->GetIps();
    for(const std::string& ip : ips){
       if(recived_set.find(ip) == recived_set.end()){
            NetNode*temp = master.manager.GetNodeByIp(ip);
            if(temp){ 
                temp->is_active = false;
                errors::Msg("ALARM: %s is currently unavailable", 
                    temp->ipv4_address.c_str());
            }
       } 
    }
    master.manager.Change();
    reciver->SetEnd();
    return true;
}

AvailabilityPinger::AvailabilityPinger(Scheduler& m, std::vector<std::string> ips_to_check)
    : InternalPinger(m)
{
    ips = ips_to_check;
}
