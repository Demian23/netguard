#include "../include/scheduler.h"

Scheduler::Scheduler(EventSelector& sel, NodesManager& m)
    : manager(m), selector(sel), descriptor(2), is_end(false)
{}

void Scheduler::AddOrdinaryTask(Task *t){schedule.push(t);}
void Scheduler::TakeOffOrdinaryTask()
{
    delete schedule.front();
    schedule.pop();
}

void Scheduler::OnTimeout()
{
    bool done = false;
    if(!schedule.empty()){
        done = schedule.front()->Execute();
        if(done)
            TakeOffOrdinaryTask();
    } else {
        selector.EndSelecting();
    }
}
void Scheduler::OnAnyEvent(){}
void Scheduler::OnError(){is_end = true;}
void Scheduler::OnRead(){}
void Scheduler::OnWrite(){}
bool Scheduler::End() const {return is_end;}
short Scheduler::ListeningEvents() const {return Timeout;}
void Scheduler::ResetEvents(int events){}
int Scheduler::GetDescriptor()const{return 2;}

Scheduler::~Scheduler()
{
    while(!schedule.empty()){
        delete schedule.front();
        schedule.pop();
    }
}

void Scheduler::AddToSelector(IEvent *e){selector.AddEvent(e);}
