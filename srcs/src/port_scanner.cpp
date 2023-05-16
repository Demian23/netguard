#include "../include/port_scanner.h"
#include "../include/errors.h"
#include "../include/ip.h"
#include <sys/socket.h>

namespace{

int create_tcp_socket()
{
    int res = 0; 
    res = socket(AF_INET, SOCK_STREAM, 0);
    if(res == -1)
        errors::Sys("tcp socket creation fail");
    return res;
}

void connect_process(int sd, sockaddr_in* dest_addr, PortCondition& cond)
{
   if(-1 != connect(sd, (sockaddr*)dest_addr, sizeof(sockaddr_in))){
       cond = Open; 
       shutdown(sd, SHUT_RDWR);
   } else {
        if(errno == ECONNREFUSED){
            cond = Closed;
            errors::Msg("%d closed", ntohs(dest_addr->sin_port));
        } else 
            errors::SysRet("");
    }
}

};

class Scanner : public IEvent{
    int fd;
    sockaddr_in dest_addr;
    PortCondition cond;
    bool end;
public:
    Scanner(sockaddr_in dest) : dest_addr(dest), cond(Unset), end(false){
        fd = create_tcp_socket();
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
    uint16_t GetDestPort()const {return ntohs(dest_addr.sin_port);}
    PortCondition GetPortCond() const {return cond;}
    void SetEnd(){end = true;}
    virtual ~Scanner(){close(fd);}
};

void Scanner::OnAnyEvent()
{
    connect_process(fd, &dest_addr, cond);
    if(cond == Unset)
       cond = Filtered; 
}

PortScanner::PortScanner(Scheduler& a_master, ports_storage& a_ports, 
    const std::string& dest_ip, Statistic* stat) 
    : master(a_master), ports(a_ports), statistic(stat)
{
    inet_aton(dest_ip.c_str(), &dest.sin_addr);
    dest.sin_family = AF_INET;
    ports_it = ports.begin();
    scanners = new Scanner*[scanners_size];
    for(int i = 0; ports_it != ports.end() && i < scanners_size; ports_it++, i++){
        dest.sin_port = htons(ports_it->first);
        Scanner* new_scanner = new Scanner(dest);
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
            if(scanners[i]->GetPortCond() != Unset){
                ports[scanners[i]->GetDestPort()] = scanners[i]->GetPortCond();
                scanners[i]->SetEnd();
                if(ports_it != ports.end()){
                    dest.sin_port = htons(ports_it->first);
                    scanners[i] = new Scanner(dest);
                    master.AddToSelector(scanners[i]);
                    ports_it++;
                    statistic->RecordStatistic(this);
                } else {
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
    }
    return result;
}

int PortScanner::GetCurrentCount()const
{
    return std::distance(ports.begin(), ports_it);
}
