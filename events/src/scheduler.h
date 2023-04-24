#ifndef SCHEDULER_DEF
#define SCHEDULER_DEF

#include "event_selector.h"
#include "fd_handlers.h"
#include "../../net_guard/src/NetDevice.h"
#include <queue>
#include <set>
#include <string>


class ScheduledEvent;
//Scheduler must have on descriptor log file
class Scheduler : public FdHandler{
public:
    Scheduler(int logfd, EventSelector& sel, Info& inf)
        : FdHandler(logfd, false), selector(sel), information(inf)
        , timeout_counter(0)
    {SetEvents(Timeout + ErrEvent);}
    bool AddHandler(FdHandler* h){return selector.Add(h);} 
    void UpdateHandlerEvents(FdHandler* h){return selector.UpdateEvents(h);}
    void EndNormalScheduledEvent();
    void SetNormalScheduledEvent(ScheduledEvent* sch_event);
    std::vector<NetDevice>& GetDevices(){return information.devices;} 
    virtual int HandleRead();
    virtual int HandleWrite();
    virtual int HandleError();
    virtual int HandleTimeout();
private:
    EventSelector& selector;
    std::queue<ScheduledEvent*> schedule;
    Info& information;
    int timeout_counter;
};

class ScheduledEvent{
public:
    virtual void Act() = 0;
    ScheduledEvent(Scheduler& m) : master(m){}
    virtual ~ScheduledEvent(){}
protected:
    Scheduler& master;
};

#endif // !SCHEDULER_DEF
