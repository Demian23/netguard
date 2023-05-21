#ifndef PINGER_DEF
#define PINGER_DEF

#include "../include/scheduler.h"
#include "../include/ip.h"
class RecvEcho;
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
    RecvEcho* reciver;
    int send_in_time;
    bool SendEcho();
    void UpdateDevices();
};

class NodesAvailabilityChecker final : public Task{
public:
    NodesAvailabilityChecker(Scheduler& m, std::vector<std::string> ips_to_check);
    bool Execute() override;
private:
    Scheduler& master;
    const std::vector<std::string> check_ips;
    std::vector<std::string>::const_iterator it;
    RecvEcho* reciver;
    int send_sd;
    int send_in_time;
    void UpdateActiveDevices();
};

#endif // !PINGER_DEF
