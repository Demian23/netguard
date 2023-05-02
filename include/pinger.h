#ifndef PINGER_DEF
#define PINGER_DEF

#include "../include/scheduler.h"
#include "../include/ip.h"

class SendEcho; class RecvEcho;
class Pinger : public ScheduledEvent{
public:
    Pinger(Scheduler& m, const std::set<std::string>& ip_set);
    virtual void Act();
    virtual ~Pinger();
    void UpdateHandlerEvents(FdHandler* h);
    void GetWrongIdPacket(char *msg, ssize_t len);
private:
    const std::set<std::string>& ping_ips;
    std::set<std::string>::const_iterator it;
    SendEcho** send_handlers; 
    int* send_icmp_sd;
    RecvEcho* reciver;
    int poll_size;
    int id;
    bool HandlersDone()const;
    bool CreateHandlers();
    void GainResults();
    void UpdateDevices();
};

#endif // !PINGER_DEF
