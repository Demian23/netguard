#ifndef PINGER_DEF
#define PINGER_DEF

#include "../include/scheduler.h"
#include "../include/ip.h"

class Pinger : public Task{
public:
    enum Mode{Slow, User};
    Pinger(Scheduler& m, Statistic* stat, Mode mode = User);
    bool Execute() override;
    inline int GetIpsSize() const{return ips_set.size();}
    int GetCurrentCount() const;
    virtual ~Pinger();
private:
    Scheduler& master;
    Statistic* statistic;
    const std::set<std::string>& ips_set;
    std::set<std::string>::const_iterator it;
    int* send_icmp_sd;
    IEvent* reciver;
    int send_in_time;
    bool SendEcho();
    void UpdateDevices();
};

#endif // !PINGER_DEF
