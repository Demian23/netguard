#ifndef SCHEDULER_DEF
#define SCHEDULER_DEF

#include "../include/event_selector.h"
#include "../include/NetDevice.h"
#include "event.h"
#include <queue>
#include <set>
#include <string>

class Task{
public:
    virtual bool Execute() = 0;
    virtual ~Task(){}
};

class Scheduler : public IEvent{
public:
    Scheduler(EventSelector& sel, NetInfo& inf);
    void AddOrdinaryTask(Task* t);
    void TakeOffOrdinaryTask();
    void OnTimeout() override;
    void OnRead() override;
    void OnWrite() override;
    void OnError() override;
    void OnAnyEvent() override;
    bool End() const override;
    short ListeningEvents() const override; 
    void ResetEvents(int events) override;
    int GetDescriptor()const override;
    virtual ~Scheduler();
    NetInfo& GetDevStat();
    void AddToSelector(IEvent* e);
private:
    EventSelector& selector;
    std::queue<Task*> schedule;
    NetInfo& dev_stat;
    int descriptor;
    bool is_end;
};


#endif // !SCHEDULER_DEF
