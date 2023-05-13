#include "../../srcs/include/host_addr.h"
#include "../../srcs/include/pinger.h"
#include "../../srcs/include/ip.h"
#include "../../srcs/include/arper.h"
#include "../../srcs/include/router.h"
#include "gui_engine.h"

#include "gui.h"

class PingerStatistic : public Statictic{
public:
    PingerStatistic(Fl_Progress *progress):progress_bar(progress){}
    void RecordStatistic(Task *task) override;
    virtual ~PingerStatistic(){progress_bar->value(progress_bar->minimum());}
private:
    Fl_Progress* progress_bar;
};

void PingerStatistic::RecordStatistic(Task *task)
{
    Pinger* p = static_cast<Pinger*>(task);
    float percent = (float)p->GetCurrentCount() * 100 / p->GetIpsSize();
    progress_bar->value(percent);
    Fl::awake();
}

class UpdateNodes : public Task{
public:
    UpdateNodes(NetGuardUserInterface* n) : interface(n){}
    bool Execute()override{interface->updateNodesBrowser(); return true;}
private: 
    NetGuardUserInterface* interface;
};

void init_interface_choices(Fl_Choice *choice)
{
    host_addr::interface_map interfaces = host_addr::get_net_interfaces();
    choice->clear(); 
    for(host_addr::interface_map::iterator it = interfaces.begin();
            it != interfaces.end(); it++){
        choice->add(it->first.c_str());
    }
}

// passing NetGuardUserInterface as data
void clbk_choice_interface(Fl_Widget* w, void* data)
{
    NetGuardUserInterface* n = reinterpret_cast<NetGuardUserInterface*>(data); 
    std::string interface = n->choice_interface->text();
    host_addr::interface_map interfaces = host_addr::get_net_interfaces();
    host_addr::interface_map::const_iterator it = interfaces.find(interface);
    if(it != interfaces.end()){
        n->out_own_mac->value(it->second.mac.c_str());
        n->out_own_net->value(it->second.net.c_str());
        n->out_own_mask->value(it->second.mask.c_str());
    } else {
        fl_alert("%s now is off.", interface.c_str());
        init_interface_choices(n->choice_interface); 
    }
}

void clbk_full_scan(Fl_Widget *w, void *data)
{
    NetGuardUserInterface* n = reinterpret_cast<NetGuardUserInterface*>(data); 
    n->schedule->manager.SetInterface(n->choice_interface->text());
    n->schedule->manager.SetIps(IP::all_net_ipv4(IP::ipv4_net(n->out_own_net->value(), n->out_own_mask->value()), 0, 
        IP::ip_amount(IP::mask_prefix(n->out_own_mask->value()))));
    n->schedule->AddOrdinaryTask(new Pinger(*n->schedule, new PingerStatistic(n->progress)));
    n->schedule->AddOrdinaryTask(new UpdateNodes(n));
    n->schedule->WakeUp();
}

void clbk_main_window(Fl_Widget* w, void* data)
{
    switch(fl_choice("Do you wanna leave?", "Yep", "No, mistake", 0)){
        case 0: {
            NetGuardUserInterface* n = reinterpret_cast<NetGuardUserInterface*>(data); 
            n->schedule->EndSchedulingAndSelecting();
            n->main_window->hide();
            break;
                }
        case 1: 
              break;
        
    }
}
