#ifndef PORT_SCANNER_DEF
#define PORT_SCANNER_DEF

#include "scheduler.h"
#include "raw_packets.h"

class PortScanner final : public Task{
public:
    enum ScanMode{All};
    PortScanner(Scheduler& a_master, const std::string& src_ip, 
        const std::string& dest_ip, ScanMode mode = All);
    bool Execute() override;
    virtual ~PortScanner();
private:
    enum{send_in_time = 25, default_src_port = 53545};
    Scheduler& master;
    IEvent* reciever;
    ports_storage ports;
    ports_storage::iterator ports_iterator; 
    std::vector<uint16_t> last_sended;
    sockaddr_in src;
    sockaddr_in dest;
    int sd;
    ports_storage GeneratePorts(ScanMode mode); 
    int RepeatSynRequests();
};

#endif // !PORT_SCANNER_DEF
