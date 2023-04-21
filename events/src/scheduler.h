#ifndef SCHEDULER_DEF
#define SCHEDULER_DEF

#include "event_selector.h"
#include "fd_handlers.h"
#include "../../net_guard/src/NetDevice.h"
#include <queue>
#include <string>


class NetAct;
//Scheduler must have on descriptor log file
class Scheduler : public FdHandler{
public:
    Scheduler(EventSelector& sel, const std::vector<NetDevice>& start_devices)
        : FdHandler(-1, false), selector(sel), devices(start_devices)
    {SetEvents(Timeout + ErrEvent);}
private:
    EventSelector& selector;
    std::queue<NetAct*> schedule;
    std::vector<NetDevice> devices;
};

class NetAct{
public:
    virtual void Act() = 0;
protected:
    NetAct(Scheduler& m) : master(m){}
    Scheduler& master;
    virtual ~NetAct(){}
};

#endif // !SCHEDULER_DEF
