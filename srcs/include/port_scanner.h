#ifndef PORT_SCANNER_DEF
#define PORT_SCANNER_DEF

#include "scheduler.h"
#include "raw_packets.h"

class Scanner;
class PortScanner final : public UrgentTask{
public:
    PortScanner(Scheduler& a_master, const std::string& src_ip, 
        const std::string& dest_ip, ports_storage& ports_to_scan);
    bool UrgentExecute() override;
    bool Execute()override{return true;}
private:
    enum{scanners_size = 20};
    Scheduler& master;
    ports_storage& ports;
    ports_storage::iterator ports_it;
    Scanner** scanners;
    sockaddr_in src;
    sockaddr_in dest;
};

#endif // !PORT_SCANNER_DEF
