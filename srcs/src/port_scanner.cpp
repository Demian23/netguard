#include "../include/port_scanner.h"
#include "../include/errors.h"
#include "../include/ip.h"
#include <sys/socket.h>

namespace{

int create_tcp_socket(sockaddr_in& address_bind_to)
{
   int res = 0; 
   res = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(res != -1){
        int flags = fcntl(res, F_GETFL);
        fcntl(res, F_SETFL, flags | O_NONBLOCK);
       while(-1 == bind(res, (sockaddr*) &address_bind_to, sizeof(address_bind_to)))
            address_bind_to.sin_port++;
   } else 
        errors::Sys("tcp socket creation fail");
   return res;
}

void connect_process(int sd, const sockaddr_in& dest_addr, PortCondition& cond)
{
   if(-1 != connect(sd, (sockaddr*)&dest_addr, sizeof(dest_addr))){
       cond = Open; 
       shutdown(sd, SHUT_RDWR);
   } else {
        if(errno == ECONNREFUSED)
            cond = Closed;
   }
}


};

class Scanner : public IEvent{
    int fd;
    sockaddr_in dest_addr;
    uint16_t dest_port;
    short repeat_request;
    PortCondition cond;
    bool end;
    bool ready_to_change_dest_port;
public:
    Scanner(sockaddr_in src_addr, sockaddr_in dest) 
        : dest_addr(dest), dest_port(-1), repeat_request(0)
        , cond(Unset), end(false), ready_to_change_dest_port(false) 
    {
        //check ret val
        //raw_packets::make_raw_socket(fd, IPPROTO_TCP);
        fd = create_tcp_socket(src_addr);
    }
    void OnRead()override{} 
    void OnWrite()override{}
    void OnError()override{errors::Sys("Scanner fail"); end = true;}
    void OnTimeout()override{}
    void OnAnyEvent()override;
    short ListeningEvents() const override{return Any;}; 
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
    virtual ~Scanner(){close(fd);}
};

void Scanner::OnAnyEvent()
{
    if(!end && !ready_to_change_dest_port){
        if(repeat_request != 3){
            connect_process(fd, dest_addr, cond);
            repeat_request++;
        }else {
           cond = Filtered; 
           ready_to_change_dest_port = true;
        }
    }
}

PortScanner::PortScanner(Scheduler& a_master, const std::string& src_ip, 
    const std::string& dest_ip, const ports_storage& ports_to_scan, Statistic* stat) 
    : master(a_master), ports(ports_to_scan), statistic(stat)
{
    src = {.sin_family = AF_INET, .sin_addr = IP::str_to_ip(src_ip.c_str())};
    dest= {.sin_family = AF_INET, .sin_addr = IP::str_to_ip(dest_ip.c_str())};
    src.sin_port = htons(50000);
    ports_it = ports.begin();
    scanners = new Scanner*[scanners_size];
    for(int i = 0; ports_it != ports.end() && i < scanners_size; i++){
        Scanner* new_scanner = new Scanner(src, dest);
        src.sin_port++;
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
                    statistic->RecordStatistic(this);
                } else {
                    scanners[i]->SetEnd();
                    scanners[i] = 0; 
                }
            }
        } else {
            zero_scanners_counter++; 
        }
    if(zero_scanners_counter == scanners_size){
        delete statistic;
        delete[] scanners;
        result = true;
        master.manager.GetMap()[inet_ntoa(dest.sin_addr)].ports = ports;
    }
    return result;
}

int PortScanner::GetCurrentCount()const
{
    return std::distance<ports_storage::const_iterator>(ports.begin(), ports_it);
}
