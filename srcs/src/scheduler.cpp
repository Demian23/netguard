#include "../include/scheduler.h"
#include <csignal>

Scheduler::Scheduler(EventSelector& sel, NodesManager& m)
    : manager(m), selector(sel), descriptor(2), is_end(false)
{
    selector.AddEvent(this);
}

void Scheduler::WakeUp()
{
    selector.SetTimeout(200);
    signal(SIGUSR1, [](int n){});
    pthread_kill(thread_id, SIGUSR1);
}

void Scheduler::AddOrdinaryTask(Task *t){schedule.push(t);}
void Scheduler::TakeOffOrdinaryTask()
{
    delete schedule.front();
    schedule.pop();
}
void Scheduler::AddUrgentTask(UrgentTask *t){urgent_schedule.push(t);}
void Scheduler::TakeOffUrgentTask()
{
    delete urgent_schedule.front();
    urgent_schedule.pop();
}

void Scheduler::OnTimeout()
{
    bool done = false;
    if(!schedule.empty()){
        done = schedule.front()->Execute();
        if(done)
            TakeOffOrdinaryTask();
    }
}

void Scheduler::OnAnyEvent()
{
    bool done = false;
    if(!urgent_schedule.empty()){
        done = urgent_schedule.front()->UrgentExecute();
        if(done)
            TakeOffUrgentTask();
    } else {
        if(schedule.empty())
            selector.SetTimeout(-1); // go sleep
                                     // need to call method that will add new tasks or go sleep changing timeout
//            selector.EndSelecting();
    }
}
void Scheduler::OnError(){is_end = true;}
void Scheduler::OnRead(){}
void Scheduler::OnWrite(){}
bool Scheduler::End() const {return is_end;}
short Scheduler::ListeningEvents() const {return Timeout + Any;}
void Scheduler::ResetEvents(int events){}
int Scheduler::GetDescriptor()const{return 2;}

Scheduler::~Scheduler()
{
    while(!schedule.empty()){
        delete schedule.front();
        schedule.pop();
    }
    while(!urgent_schedule.empty()){
        delete urgent_schedule.front();
        urgent_schedule.pop();
    }
}

void Scheduler::AddToSelector(IEvent *e){selector.AddEvent(e);}
//prepare to end
void Scheduler::EndSchedulingAndSelecting(){WakeUp();selector.EndSelecting();}
