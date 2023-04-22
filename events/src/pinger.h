#ifndef PINGER_DEF
#define PINGER_DEF

#include "scheduler.h"
#include "../../net/src/ip.h"

class SendEcho; class RecvEcho;
class Pinger : public ScheduledEvent{
public:
    Pinger(Scheduler& m, const std::set<std::string>& ip_set);
    virtual void Act();
    virtual ~Pinger();
    void UpdateHandlerEvents(FdHandler* h);
private:
    const std::set<std::string>& ping_ips;
    std::set<std::string>::const_iterator it;
    std::vector<std::string> real_ip;
    SendEcho** send_handlers; 
    RecvEcho** recieve_hanlders; 
    int* send_icmp_sd;
    int* recieve_icmp_sd; 
    int poll_size;
    int id;
    bool HandlersDone()const;
    bool CreateHandlers();
    void GainResults();
    void UpdateDevices();
};

#endif // !PINGER_DEF
