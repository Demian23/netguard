#include "../include/port_scanner.h"
#include "../include/errors.h"
#include "../include/ip.h"

class Scanner : public IEvent{
    int fd;
    sockaddr_in own_addr;
    sockaddr_in dest_addr;
    uint16_t src_port;
    uint16_t dest_port;
    short repeat_request;
    PortCondition cond;
    bool end;
    bool ready_to_change_dest_port;
public:
    Scanner(uint16_t src_port, sockaddr_in src_addr, sockaddr_in dest) 
        : own_addr(src_addr), dest_addr(dest), dest_port(-1), repeat_request(0)
        , cond(Unset), end(false), ready_to_change_dest_port(false) 
    {
        //check ret val
        raw_packets::make_raw_socket(fd, IPPROTO_TCP);
        int flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    void OnRead()override; 
    void OnWrite()override;
    void OnError()override{errors::Sys("Scanner fail"); end = true;}
    void OnTimeout()override{}
    void OnAnyEvent()override{}
    short ListeningEvents() const override{return Read + Write;}; 
    void ResetEvents(int events)override{}
    int GetDescriptor()const override{return fd;}
    bool End() const override{return end;}
    void ChangeDestPort(uint16_t p){
        dest_port = p; cond = Unset; 
        ready_to_change_dest_port = false; repeat_request = false;
        repeat_request = 0;
    }
    uint16_t GetDestPort()const {return dest_port;}
    bool IsReady()const {return ready_to_change_dest_port;}
    PortCondition GetPortCond() const {return cond;}
    void SetEnd(){end = true;}
};

void Scanner::OnWrite()
{
    if(!end && !ready_to_change_dest_port){
        if(repeat_request != 3){
            raw_packets::send_tcp_flag(fd, own_addr, &dest_addr, src_port, dest_port, TH_SYN); 
            repeat_request++;
        }else {
           cond = Filtered; 
           ready_to_change_dest_port = true;
        }
    }
}

void Scanner::OnRead()
{
    bool end_read;
    sockaddr_in from;
    do{
        char read_buffer[1024] = {};
        ssize_t len = raw_packets::recieve_packet(fd, read_buffer, 1024, &from);
        end_read = len == -1;
        if(!end_read){
            bool syn_res = raw_packets::get_syn_answer(read_buffer, len, &from);
            if(from.sin_addr.s_addr == dest_addr.sin_addr.s_addr
                && from.sin_port == dest_port){
                if(syn_res){
                    cond = Open;
                    raw_packets::send_tcp_flag(fd, own_addr, &dest_addr, src_port, dest_port, TH_RST);
                } else 
                    cond = Closed;
            }
        }
    }while(!end_read);
}

PortScanner::PortScanner(Scheduler& a_master, const std::string& src_ip, 
    const std::string& dest_ip, ports_storage& ports_to_scan) 
    : master(a_master), ports(ports_to_scan)
{
    src = {.sin_family = AF_INET, .sin_addr = IP::str_to_ip(src_ip.c_str())};
    dest= {.sin_family = AF_INET, .sin_addr = IP::str_to_ip(dest_ip.c_str())};
    ports_it = ports.begin();
    scanners = new Scanner*[scanners_size];
    uint16_t src_port = 53322; // find free port
    for(int i = 0; ports_it != ports.end() && i < scanners_size; i++){
        Scanner* new_scanner = new Scanner(src_port, src, dest);
        src_port++;
        new_scanner->ChangeDestPort(ports_it->first);
        scanners[i] = new_scanner;
        master.AddToSelector(new_scanner);
    }
}

bool PortScanner::UrgentExecute()
{
    bool result = false;    
    int zero_scanners_counter = 0;
    for(int i = 0; i < scanners_size; i++)
        if(scanners[i]){
            if(scanners[i]->IsReady()){
                ports[scanners[i]->GetDestPort()] = scanners[i]->GetPortCond();
                if(ports_it != ports.end()){
                    scanners[i]->ChangeDestPort(ports_it->first);
                    ports_it++;
                } else {
                    scanners[i]->SetEnd();
                    scanners[i] = 0; 
                }
            }
        } else {
           zero_scanners_counter++; 
        }
    if(zero_scanners_counter == scanners_size){
       delete[] scanners;
       result = true;
    }
    return result;
}
