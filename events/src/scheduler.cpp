#include "scheduler.h"

void Scheduler::EndNormalScheduledEvent()
{
    delete schedule.front();
    schedule.pop(); 
}

void Scheduler::SetNormalScheduledEvent(ScheduledEvent *sch_event)
{
    schedule.push(sch_event);
}

int Scheduler::HandleTimeout()
{
    if(!schedule.empty())
        schedule.front()->Act();    
    else{
        selector.EndRun();
    }
    return 0;
}

int Scheduler::HandleRead(){return 0;}
int Scheduler::HandleWrite(){return 0;}
int Scheduler::HandleError(){return 0;}
