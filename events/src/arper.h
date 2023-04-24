#ifndef ARPER_DEF
#define ARPER_DEF


#include "scheduler.h"
class ARPHandler;
class Arper : public ScheduledEvent{
public:
    Arper(Scheduler& m, const std::string& a_in);
    virtual void Act();
    virtual ~Arper(){}
    void UpdateHandlerEvents(FdHandler* h);
private:
    std::vector<NetDevice> updated_dev;
    const std::string& interface;
    NetDevice own_dev;
    int current_index;
    bool init;

    void Init();
};

#endif 
