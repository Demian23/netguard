#include "../include/port_scanner.h"
#include "../include/errors.h"
#include "../include/ip.h"

class SynReciver final : public IEvent{
private:
    enum{buffer_size = 1024};
    ports_storage& ports;
    char* buffer;
    int fd;
    bool end;
public:
    SynReciver(ports_storage& some_ports);
    void OnRead()override; 
    void OnWrite()override{}
    void OnError()override{errors::SysRet(""); end = true;}
    void OnTimeout()override{}
    void OnAnyEvent()override{}
    short ListeningEvents() const override{return Read + Error;} 
    void ResetEvents(int events)override{}
    int GetDescriptor()const override{return fd;}
    bool End() const override{return end;}
    void SetEnd(){end = true;}
    virtual ~SynReciver(){close(fd); delete[] buffer;}
};

SynReciver::SynReciver(ports_storage& some_ports) : ports(some_ports)
{
    //exception
    raw_packets::make_raw_socket(fd, IPPROTO_IP);
//    if(!raw_packets::bind_socket_to_interface(fd, 
//        "en0"))
//        errors::Dump("");
    buffer = new char[buffer_size];
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void SynReciver::OnRead()
{
    bool end_read;
    sockaddr_in from;
    from.sin_addr.s_addr = 0;
    do{
        ssize_t len = raw_packets::recieve_packet(fd, buffer, buffer_size, &from);
        end_read = len == -1;
        if(!end_read){
            if(raw_packets::get_syn_answer(buffer, len, &from)){
                uint16_t key = ntohs(from.sin_port);
                if(ports.find(key) != ports.end())
                    ports[key] = Open;
            } else {
                if(from.sin_addr.s_addr != 0){
                    uint16_t key = ntohs(from.sin_port);
                    if(ports.find(key) != ports.end())
                        ports[key] = Closed;
                }
            }
        }
    }while(!end_read);
}

PortScanner::PortScanner(Scheduler& a_master, const std::string& src_ip, const std::string& dest_ip, 
    ScanMode mode) : master(a_master)
{
    ports = GeneratePorts(mode);    
    ports_iterator = ports.begin();
    // exeption
    raw_packets::make_raw_socket(sd, IPPROTO_TCP);
    dest.sin_addr = IP::str_to_ip(dest_ip.c_str());
    dest.sin_family = AF_INET;
    src.sin_addr = IP::str_to_ip(src_ip.c_str());
    src.sin_family = AF_INET;
}

ports_storage PortScanner::GeneratePorts(ScanMode mode)
{
    ports_storage m; 
    if(mode == All){
        m.reserve(1000);
        for(int i = 0; i < 1000; i++)
            m[i] = PortCondition::Unset;  // is it necessery?
    } else {
        errors::Msg("No such port scanning mode");
    }
    return m;
}

int PortScanner::RepeatSynRequests()
{
    int i = 0; 
    std::vector<uint16_t>::iterator it = last_sended.begin();
    while(it != last_sended.end()){
        if(ports[last_sended[i]] == Unset){
           raw_packets::send_tcp_flag(sd, src, 
                &dest, default_src_port, last_sended[i], TH_SYN); 
           ports[last_sended[i]] = Repeat;
           i++;
           it++;
        } else {
            if(ports[last_sended[i]] == Repeat)
                ports[last_sended[i]] = Filtered;
            if(ports[last_sended[i]] == Open)
               raw_packets::send_tcp_flag(sd, src, 
                    &dest, default_src_port, last_sended[i], TH_RST); 
            it = last_sended.erase(it);
        }
    }
    return i;
}

bool PortScanner::Execute() 
{
    if(reciever == 0){
        reciever = new SynReciver(ports);
        master.AddToSelector(reciever);
    }
    int i = RepeatSynRequests();
    bool result = false;
    if(ports_iterator != ports.end())
        for(;i < send_in_time && ports_iterator != ports.end(); i++, ports_iterator++){
           raw_packets::send_tcp_flag(sd, src, &dest, default_src_port, 
                ports_iterator->first, TH_SYN); 
           last_sended.push_back(ports_iterator->first);
        }
    if(i == 0){
        static_cast<SynReciver*>(reciever)->SetEnd();
        std::string temp = inet_ntoa(dest.sin_addr);
        if(!temp.empty()) 
            master.manager.GetMap()[temp].ports = ports;
        result = true;
    }
    return result;
}
PortScanner::~PortScanner(){close(sd);}
