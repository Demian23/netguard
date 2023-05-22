#ifndef PINGER_DEF
#define PINGER_DEF

#include "../include/scheduler.h"

class RecvEcho;
class Pinger : public Task{
public:
    Pinger(Scheduler& m);
    virtual ~Pinger();
protected:
    Scheduler& master;
    std::vector<std::string> ips;
    RecvEcho* reciver;
    int send_sd;
    int ip_pos;
    bool SendOneEcho();
    virtual bool UpdateDevices();
};

class UsrPinger : public Pinger{
public:
    UsrPinger(Scheduler& m, Statistic* stat);
    inline int GetIpsSize() const{return ips.size();}
    inline int GetCurrentCount() const{return ip_pos;}
    bool Execute() override;
    virtual ~UsrPinger();
private:
    int send_in_time;
    Statistic* statistic;
};

class InternalPinger : public Pinger{
public:
    InternalPinger(Scheduler& m) : Pinger(m){}
    bool Execute()override;
};

class AvailabilityPinger : public InternalPinger{
public: 
    AvailabilityPinger(Scheduler& m, std::vector<std::string> ips_to_check);
    bool UpdateDevices()override;
};


#endif // !PINGER_DEF
