#include "../include/scheduler.h"
#include "../include/pinger.h"
#include "../include/arper.h"
#include "../include/router.h"

void Scheduler::EndNormalScheduledEvent()
{
    delete schedule.front();
    schedule.pop(); 
    if(schedule.empty()){
        timeout_counter = 0;
        std::for_each(information.devices.begin(), information.devices.end(), 
            print); 
        selector.EndRun();
    }
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
//        timeout_counter++;
//        if(timeout_counter == 10){
            Pinger* ping = new Pinger(*this, information.ip_set);
            Arper* arp = new Arper(*this, information.interface);
            FindGate* router = new FindGate(*this);

            SetNormalScheduledEvent(ping);
            SetNormalScheduledEvent(arp);
            SetNormalScheduledEvent(router);
 //       }

    }
    return 0;
}

int Scheduler::HandleRead(){return 0;}
int Scheduler::HandleWrite(){return 0;}
int Scheduler::HandleError(){return 0;}
