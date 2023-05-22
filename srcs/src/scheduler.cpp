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
    if(selector.GetTimeout() == -1){
        selector.SetTimeout(200);
        signal(SIGUSR1, [](int n){});
        pthread_kill(thread_id, SIGUSR1);
    }
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
        if(active_mode && !internal_schedule.empty()){
            done = internal_schedule.front()->Execute();
            if(done){
                delete internal_schedule.front();
                internal_schedule.pop();
            } 
        }
    }
}

void Scheduler::OnAnyEvent()
{
    bool done = false;
    if(urgent_schedule.empty() && schedule.empty() && internal_schedule.empty()){
        if(active_mode)
            SetIternalQueue();
        else
            selector.SetTimeout(-1);
    }
    if(!urgent_schedule.empty()){
        done = urgent_schedule.front()->UrgentExecute();
        if(done)
            TakeOffUrgentTask();
    }
}
void Scheduler::OnError(){is_end = true;}
bool Scheduler::End() const {return is_end;}
short Scheduler::ListeningEvents() const {return Timeout + Any;}
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
    while(!internal_schedule.empty()){
        delete internal_schedule.front();
        internal_schedule.pop();
    }
}

void Scheduler::AddToSelector(IEvent *e){selector.AddEvent(e);}
//prepare to end
void Scheduler::EndSchedulingAndSelecting(){WakeUp();selector.EndSelecting();}

void Scheduler::TurnOnActiveMode(){WakeUp();active_mode = true;SetIternalQueue();}
void Scheduler::TurnOffActiveMode()
{
    active_mode = false; 
    while(!internal_schedule.empty()){
        delete internal_schedule.front();
        internal_schedule.pop();
    }
}

void Scheduler::SetIternalQueue()
{
    internal_schedule.push(new InternalPinger(*this));
    internal_schedule.push(new Arper(*this));
    internal_schedule.push(new FindGate(*this));
    internal_schedule.push(new AvailabilityPinger(*this, manager.GetActiveIps()));
}
