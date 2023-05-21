#ifndef PORT_SCANNER_DEF
#define PORT_SCANNER_DEF

#include "scheduler.h"
#include "raw_packets.h"
class TcpHandshakeAnalizer;
class PortScanner final : public UrgentTask{
public:
    PortScanner(Scheduler& a_master, const ports_storage& a_ports, 
        const std::string& dest_ip_str, Statistic* stat);
    bool UrgentExecute() override;
    inline int GetPortsSize() const{return ports.size();}
    int GetCurrentCount() const;
    std::string GetAim()const;
private:
    Scheduler& master;
    ports_storage ports;
    ports_storage::iterator ports_it;
    Statistic* statistic;
    std::vector<uint16_t> previous_ports;
    std::vector<TcpHandshakeAnalizer*> recivers;
    sockaddr_in src;
    sockaddr_in dest;
    int manual_sd;
    short wait_counter;
    int scanners_size;
    bool CheckPreviousPorts(); // if true, than can go on
    bool Prepare();
    bool ReleaseResources();
    void SendSyn();
};


#endif // !PORT_SCANNER_DEF
