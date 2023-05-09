#ifndef SCHEDULER_DEF
#define SCHEDULER_DEF

#include "../include/event_selector.h"
#include "../include/nodes_manager.h"
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
    Scheduler(EventSelector& sel, NodesManager& m);
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
    void AddToSelector(IEvent* e);
    NodesManager& manager;
private:
    EventSelector& selector;
    std::queue<Task*> schedule;
    int descriptor;
    bool is_end;
};


#endif // !SCHEDULER_DEF
