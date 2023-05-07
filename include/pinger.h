#ifndef PINGER_DEF
#define PINGER_DEF

#include "../include/scheduler.h"
#include "../include/ip.h"

class Pinger : public Task{
public:
    Pinger(Scheduler& m, const std::set<std::string>& ip_set);
    bool Execute() override;
    virtual ~Pinger();
private:
    enum{send_in_time = 8};
    Scheduler& master;
    std::set<std::string>::const_iterator it;
    int* send_icmp_sd;
    IEvent* reciver;
    bool SendEcho();
    void UpdateDevices();
};

#endif // !PINGER_DEF
