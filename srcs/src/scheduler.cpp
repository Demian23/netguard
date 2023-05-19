#include <unistd.h>
#include <csignal>
#include "../include/pinger.h"
#include "../include/arper.h"
#include "../include/router.h"
#include "../include/scheduler.h"

Scheduler::Scheduler(EventSelector& sel, NodesManager& m)
    : manager(m), selector(sel), is_end(false), active_mode(false)
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
    }else{
        if(active_mode && !iternal_schedule.empty()){
            done = iternal_schedule.front()->Execute();
            if(done){
                delete iternal_schedule.front();
                iternal_schedule.pop();
            } 
        }
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
        if(schedule.empty()){
            if(active_mode){
                if(iternal_schedule.empty()){
                    SetIternalQueue();
                }
            }
            else 
                selector.SetTimeout(-1);     
        }
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
    close(2);
}

void Scheduler::AddToSelector(IEvent *e){selector.AddEvent(e);}
//prepare to end
void Scheduler::EndSchedulingAndSelecting(){WakeUp();selector.EndSelecting();}

void Scheduler::TurnOnActiveMode(){active_mode = true;SetIternalQueue();}
void Scheduler::TurnOffActiveMode()
{
    active_mode = false; 
    while(!iternal_schedule.empty()){
        delete iternal_schedule.front();
        iternal_schedule.pop();
    }
}

void Scheduler::SetIternalQueue()
{
    iternal_schedule.push(new Pinger(*this, 0, Pinger::Slow));
    iternal_schedule.push(new Arper(*this));
    iternal_schedule.push(new FindGate(*this));
}
