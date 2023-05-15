#ifndef PORT_SCANNER_DEF
#define PORT_SCANNER_DEF

#include "scheduler.h"
#include "raw_packets.h"

class Scanner;
class PortScanner final : public UrgentTask{
public:
    PortScanner(Scheduler& a_master, const std::string& src_ip, 
        const std::string& dest_ip, Statistic* stat);
    bool UrgentExecute() override;
    bool Execute()override{return true;}
    inline int GetPortsSize() const{return master.manager.GetMap()[dest_ip_str].ports.size();}
    int GetCurrentCount() const;
private:
    enum{scanners_size = 20};
    Scheduler& master;
    ports_storage::iterator ports_it;
    Scanner** scanners;
    Statistic* statistic;
    std::string dest_ip_str;
    sockaddr_in src;
    sockaddr_in dest;
};

#endif // !PORT_SCANNER_DEF
