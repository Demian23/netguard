#ifndef PINGER_DEF
#define PINGER_DEF

#include "scheduler.h"
#include "icmp_handlers.h"

class Pinger : public NetAct{
public:
    Pinger(Scheduler& m, const std::string& own_ip, const std::string& net, short mask_prefix);
    virtual void Act();
    virtual ~Pinger();
private:
    char** ping_ips;
    SendEcho** send_handlers; 
    RecvEcho** recieve_hanlders; 
    int* send_icmp_sd;
    int* recieve_icmp_sd; 
    int ping_interval_size;
    int poll_size;
    int id;
    int current_ip_index;
};

#endif // !PINGER_DEF
