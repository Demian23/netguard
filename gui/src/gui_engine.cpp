#include "../../srcs/include/host_addr.h"
#include "../../srcs/include/pinger.h"
#include "../../srcs/include/ip.h"
#include "../../srcs/include/mac.h"
#include "../../srcs/include/arper.h"
#include "../../srcs/include/router.h"
#include "../../srcs/include/port_scanner.h"
#include "gui_engine.h"

#include "gui.h"

class PingerStatistic : public Statistic{
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

class PortScannerStatistic : public Statistic{
public:
    PortScannerStatistic(Fl_Progress* port_progress) : port_progress_bar(port_progress){}
    void RecordStatistic(Task* task)override;
    virtual ~PortScannerStatistic(){port_progress_bar->value(port_progress_bar->minimum());}
private:
    Fl_Progress* port_progress_bar;
};

void PortScannerStatistic::RecordStatistic(Task *task)
{
    PortScanner* p = static_cast<PortScanner*>(task);
    float percent = (float)p->GetCurrentCount() * 100 / p->GetPortsSize();
    port_progress_bar->value(percent);
    Fl::awake();
}

class UpdateNodes : public Task{
public:
    UpdateNodes(NetGuardUserInterface* n) : interface(n){}
    bool Execute()override{interface->updateNodesBrowser(); return true;}
private: 
    NetGuardUserInterface* interface;
};

class UpdatePorts : public UrgentTask{
public:
    UpdatePorts(NetGuardUserInterface* n, const std::string& destination)
        : interface(n), dest(destination){}
    bool UrgentExecute()override{interface->updatePortsBrowser(dest);return true;}
    bool Execute()override{return true;}
private:
    NetGuardUserInterface* interface;
    const std::string& dest;
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
        n->out_own_ip->value(it->second.ip.c_str());
        n->out_own_mask->value(it->second.mask.c_str());
        std::string net = IP::ipv4_net(it->second.ip, it->second.mask);
        n->out_net->value(net.c_str());
        n->first_ip->value(IP::first_ip(net).c_str());
        n->last_ip->value(IP::last_ip(net, it->second.mask).c_str());
    } else {
        fl_alert("%s now is off.", interface.c_str());
        init_interface_choices(n->choice_interface); 
    }
}

void clbk_full_scan(Fl_Widget *w, void *data)
{
    NetGuardUserInterface* n = reinterpret_cast<NetGuardUserInterface*>(data); 
    n->schedule->manager.SetInterface(n->choice_interface->text());
    std::set<std::string> ip_set = 
        IP::all_ipv4_from_range(n->first_ip->value(),n->last_ip->value());
    NetNode own_device;
    own_device.ipv4_address = n->out_own_ip->value();
    own_device.type = "Own host";
    own_device.name = host_addr::get_own_name();
    own_device.mac_address = n->out_own_mac->value();
    own_device.is_active = true;
    ether_addr* temp = ether_aton(own_device.mac_address.c_str());
    own_device.vendor = MAC::get_vendor(*temp);
    ip_set.erase(own_device.ipv4_address);
    n->schedule->manager.SetIps(ip_set);
    n->schedule->manager.AddNode(own_device);
    n->schedule->AddOrdinaryTask(new Pinger(*n->schedule, new PingerStatistic(n->progress)));
    n->schedule->AddOrdinaryTask(new Arper(*n->schedule));
    n->schedule->AddOrdinaryTask(new FindGate(*n->schedule));
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

void clbk_nodes_brws(Fl_Widget *w, void *data)
{
    NetGuardUserInterface* n = reinterpret_cast<NetGuardUserInterface*>(data); 
    int line = n->brws_nodes->value();
    if(line){
        void* node_data = n->brws_nodes->data(line);
        NetNode* node = reinterpret_cast<NetNode*>(node_data);
        n->out_ip->value(node->ipv4_address.c_str());
        n->out_mac->value(node->mac_address.c_str());
        n->out_vendor->value(node->vendor.c_str());
        n->out_name->value(node->name.c_str());
        n->out_type->value(node->type.c_str());
    }
}

void clbk_port_scan(Fl_Widget* w, void *data)
{
    NetGuardUserInterface* n = reinterpret_cast<NetGuardUserInterface*>(data); 
    std::string dest_ip = n->out_ip->value();
    std::string src_ip = n->out_own_ip->value();
    NetMap& temp = n->schedule->manager.GetMap();
    n->schedule->AddUrgentTask(new PortScanner(*n->schedule, src_ip, dest_ip, 
        temp[dest_ip].ports, new PortScannerStatistic(n->ports_scan_progress)));
    n->schedule->AddUrgentTask(new UpdatePorts(n, dest_ip));
    n->schedule->WakeUp();
}

