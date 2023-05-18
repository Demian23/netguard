#ifndef SCHEDULER_DEF
#define SCHEDULER_DEF

#include "../include/event_selector.h"
#include "../include/nodes_manager.h"
#include "event.h"
#include <queue>
#include <set>
#include <string>
#include <pthread.h>

class Task{
public:
    virtual bool Execute(){return true;};
    virtual ~Task(){}
};

// task and urgent task should be never mixed
class UrgentTask : public Task{
public:
    virtual bool UrgentExecute(){return true;}
    virtual ~UrgentTask(){}
};

class Statistic{
public:
    virtual void RecordStatistic(Task* task) = 0;
    virtual ~Statistic(){}
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
    void SetThreadId(pthread_t id){thread_id = id;}
    void TurnOnActiveMode();
    void TurnOffActiveMode();
    NodesManager& manager;
private:
    EventSelector& selector;
    std::queue<Task*> schedule;
    std::queue<UrgentTask*> urgent_schedule;
    std::queue<Task*> iternal_schedule;
    pthread_t thread_id;
    int descriptor;
    bool is_end;
    bool active_mode;
    void SetIternalQueue();
};


#endif // !SCHEDULER_DEF
