#ifndef PINGER_DEF
#define PINGER_DEF

#include "../include/scheduler.h"
#include "../include/ip.h"

class Pinger : public Task{
public:
    Pinger(Scheduler& m, Statictic* stat);
    bool Execute() override;
    inline int GetIpsSize() const{return ips_set.size();}
    int GetCurrentCount() const;
    virtual ~Pinger();
private:
    enum{send_in_time = 8};
    Scheduler& master;
    Statictic* statistic;
    const std::set<std::string>& ips_set;
    std::set<std::string>::const_iterator it;
    int* send_icmp_sd;
    IEvent* reciver;
    bool SendEcho();
    void UpdateDevices();
};

#endif // !PINGER_DEF
