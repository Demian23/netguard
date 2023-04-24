#ifndef ROUTER_DEF
#define ROUTER_DEF

#include "scheduler.h"
class FindGate: public ScheduledEvent{
public:
    FindGate(Scheduler& m) : ScheduledEvent(m){}
    virtual void Act();
    virtual ~FindGate(){}
};

#endif // !ROUTER_DEF
