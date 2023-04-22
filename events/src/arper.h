#ifndef ARPER_DEF
#define ARPER_DEF


#include "scheduler.h"
class ARPHandler;
class Arper : public ScheduledEvent{
public:
    Arper(Scheduler& m);
    virtual void Act();
    virtual ~Arper();
    void UpdateHandlerEvents(FdHandler* h);
private:
    std::vector<NetDevice> updated_dev;
};

#endif 
