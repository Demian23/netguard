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

// task and urgent task should be never mixed
class UrgentTask : public Task{
public:
    virtual bool UrgentExecute() = 0;
    virtual ~UrgentTask(){}
};

class Statictic{
public:
    virtual void RecordStatistic(Task* task) = 0;
    virtual ~Statictic(){}
};

class Scheduler : public IEvent{
public:
    Scheduler(EventSelector& sel, NodesManager& m);
    void AddOrdinaryTask(Task* t);
    void TakeOffOrdinaryTask();
    void AddUrgentTask(UrgentTask* t);
    void TakeOffUrgentTask();
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
    void WakeUp();
    void EndSchedulingAndSelecting();
    NodesManager& manager;
private:
    EventSelector& selector;
    std::queue<Task*> schedule;
    std::queue<UrgentTask*> urgent_schedule;
    int descriptor;
    bool is_end;
};


#endif // !SCHEDULER_DEF
