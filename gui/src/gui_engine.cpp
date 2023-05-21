#include "../../srcs/include/host_addr.h"
#include "../../srcs/include/pinger.h"
#include "../../srcs/include/ip.h"
#include "../../srcs/include/mac.h"
#include "../../srcs/include/arper.h"
#include "../../srcs/include/router.h"
#include "../../srcs/include/port_scanner.h"
#include "../../srcs/include/errors.h"
#include "gui_engine.h"
#include "gui.h"

class GuiUpdater final : public IEvent{
    static bool created;
    NetGuardUserInterface& interface;
    NodesManager& manager;
    int fd;
    bool end;
    GuiUpdater(NetGuardUserInterface& n, NodesManager& m) 
        : interface(n), manager(m), end(false){fd = open("netguard.log", O_RDONLY);}
public:
    void OnError()override{end = true;}
    bool End()const override{return end;}
    int GetDescriptor() const override{return fd;}
    short ListeningEvents()const override{return Any;} 
    void OnAnyEvent()override;
    virtual ~GuiUpdater(){close(fd);}
    static GuiUpdater* Make(NetGuardUserInterface& n);
};

bool GuiUpdater::created = false;

GuiUpdater* GuiUpdater::Make(NetGuardUserInterface &n)
{
    if(!created){created = true;return new GuiUpdater(n, n.schedule->manager);}
    else return 0;
}

void GuiUpdater::OnAnyEvent()
{
    if(manager.IsChanged()){
        interface.updateNodesBrowser();
        manager.Updated();
    }
    char buffer[1024] = {};
    int size = read(fd, buffer, 1024);
    if(size > 0)
        interface.log_buffer->append(buffer);
}

class PingerStatistic : public Statistic{
public:
    PingerStatistic(Fl_Progress *progress):progress_bar(progress){}
    void RecordStatistic(Task *task) override;
    void ShowMistake(std::string mistake_msg)override{fl_alert("%s", mistake_msg.c_str());}
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
    virtual ~PortScannerStatistic(){port_progress_bar->value(port_progress_bar->minimum()); port_progress_bar->label("scanning");}
private:
    Fl_Progress* port_progress_bar;
};

void PortScannerStatistic::RecordStatistic(Task *task)
{
    PortScanner* p = static_cast<PortScanner*>(task);
    float percent = (float)p->GetCurrentCount() * 100 / p->GetPortsSize();
    port_progress_bar->value(percent);
    port_progress_bar->copy_label((p->GetAim() + " scanning").c_str());
    Fl::awake();
}

class UpdatePorts : public UrgentTask{
public:
    UpdatePorts(NetGuardUserInterface* n, const std::string& destination)
        : interface(n), dest(destination){}
    bool UrgentExecute()override{interface->updatePortsBrowser(dest);return true;}
    bool Execute()override{return true;}
private:
    NetGuardUserInterface* interface;
    std::string dest;
};

PortsTable::PortsTable(int x, int y, int table_w, int table_h, const char* label,
    NodesManager& m)
    : Fl_Table_Row(x, y, table_w, table_h, label), manager(m)
{
    rows(0xFFFF);
    row_resize(0);

   cols(4);
   col_width(1, col_width(1) + 40);
   col_header(1);
   col_resize(0);
   type(SELECT_NONE);
   end();
}

void PortsTable::DrawHeader(const char *str, int X, int Y, int W, int H)
{
    fl_push_clip(X, Y, W, H);
    fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
    fl_color(FL_BLACK);
    fl_draw(str, X,Y,W,H, FL_ALIGN_CENTER);
    fl_pop_clip();
}

void PortsTable::DrawCell(const char *str, int X, int Y, int W, int H, bool select)
{
    fl_push_clip(X, Y, W, H);
    int bgcolor = select ? selection_color() : FL_WHITE;
    int textcolor = select ? FL_YELLOW : FL_BLACK;
    fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, bgcolor);
    fl_color(textcolor);
    fl_draw(str, X, Y, W, H, FL_ALIGN_LEFT);
    fl_pop_clip();
}

void PortsTable::DrawCellContent(int R, int C, int X, int Y, int W, int H)
{
    char buff[64] = {};
    if(R >= printable_ports.size())
        DrawCell("", X, Y, W, H, row_selected(R));
    else{
        switch(C){
            case 0:
                IP::itoa(printable_ports[R], buff);
                DrawCell(buff, X, Y, W, H, row_selected(R));
                break;
            case 1:
                DrawCell(manager.GetService(printable_ports[R]).c_str(), X, Y, W, H, row_selected(R));
                break;
            case 2:
                DrawCell(manager.GetProtocol(printable_ports[R]).c_str(), X, Y, W, H, row_selected(R));
                break;
            case 3:
                DrawCell(manager.GetPortCond(current_ip, printable_ports[R]), X, Y, W, H, row_selected(R));
                break;
            default: break;
        }
    }
}


void PortsTable::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H)
{
    const char *header_text[4] = {"Port", "Service", "Protocol", "Condition"};
    switch (context) {
        case CONTEXT_STARTPAGE:             // Fl_Table telling us it's starting to draw page
            fl_font(FL_HELVETICA, 18);
            return;
        case CONTEXT_COL_HEADER:
            DrawHeader(header_text[C], X, Y, W, H);
            return;
        case CONTEXT_ROW_HEADER:      
        case CONTEXT_CELL:                  // Fl_Table telling us to draw cells
            DrawCellContent(R, C, X, Y, W, H);
            return;
    default:
        return;
    }
}



void init_interface_choices(Fl_Choice *choice)
{
    host_addr::interface_map interfaces = host_addr::get_net_interfaces();
    choice->clear(); 
    for(host_addr::interface_map::iterator it = interfaces.begin();
            it != interfaces.end(); it++){
        choice->add(it->first.c_str());
        errors::Msg("Available interface: %s", it->first.c_str());
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
    if(n->choice_interface->text()){
        n->schedule->manager.SetInterface(n->choice_interface->text());
        std::string first_ip_string = n->first_ip->value();
        std::string last_ip_string = n->last_ip->value();
        if(IP::is_valid_ip_string(first_ip_string) && IP::is_valid_ip_string(last_ip_string) 
        && IP::check_ip_range(n->out_net->value(), n->out_own_mask->value(), first_ip_string, last_ip_string)){
            std::set<std::string> ip_set = 
                IP::all_ipv4_from_range(first_ip_string, last_ip_string);
            IEvent* updater = GuiUpdater::Make(*n);
            if(updater != 0)
                n->schedule->AddToSelector(updater);
            NetNode own_device;
            own_device.ipv4_address = n->out_own_ip->value();
            own_device.type = "Own host";
            own_device.name = host_addr::get_own_name();
            own_device.mac_address = n->out_own_mac->value();
            own_device.is_active = true;
            ether_addr* temp = ether_aton(own_device.mac_address.c_str());
            own_device.vendor = MAC::get_vendor(*temp);
            ip_set.erase(own_device.ipv4_address);
            n->schedule->manager.SetIpSet(ip_set);
            n->schedule->manager.AddNode(own_device);
            n->schedule->AddOrdinaryTask(new Pinger(*n->schedule, new PingerStatistic(n->progress)));
            n->schedule->AddOrdinaryTask(new Arper(*n->schedule));
            n->schedule->AddOrdinaryTask(new FindGate(*n->schedule));
            n->schedule->WakeUp();
        }else{
            n->first_ip->value(IP::first_ip(n->out_net->value()).c_str());
            n->last_ip->value(IP::last_ip(n->out_net->value(), n->out_own_mask->value()).c_str());
            fl_alert("Wrong ip range!");
        }
    } else {
        fl_alert("Choose interface!");
    }
}

void clbk_main_window(Fl_Widget* w, void* data)
{
    /*
    switch(fl_choice("Do you wanna leave?", "Yep", "No, mistake", 0)){
        case 0: {
            break;
                }
        case 1: 
              break;
        
    }
    */
    NetGuardUserInterface* n = reinterpret_cast<NetGuardUserInterface*>(data); 
    n->schedule->EndSchedulingAndSelecting();
    n->main_window->hide();
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
        n->updatePortsBrowser(node->ipv4_address);
        if(node->type != "Own host" && node->is_active)
            n->btn_ports_scan->activate();
        else 
            n->btn_ports_scan->deactivate();
    }
}

ports_storage get_ports(Fl_Check_Browser* brws)
{
    ports_storage new_ports;
    for(int i = 0; i <= 65535; i++){
        if(brws->checked(i))
            new_ports.emplace(i, Unset);
    }
    return new_ports;
}

void clbk_port_scan(Fl_Widget* w, void *data)
{
    NetGuardUserInterface* n = reinterpret_cast<NetGuardUserInterface*>(data); 
    std::string dest_ip = n->out_ip->value();
    std::string src_ip = n->out_own_ip->value();
    if(!dest_ip.empty() && n->schedule->manager.GetNodeByIp(dest_ip)->is_active){
        n->schedule->AddUrgentTask(new PortScanner(*n->schedule, 
          get_ports(n->brws_ports), dest_ip, 
            new PortScannerStatistic(n->ports_scan_progress)));
        n->schedule->AddUrgentTask(new UpdatePorts(n, dest_ip));
        n->schedule->WakeUp();
    } else {
        fl_alert("Choose node first");
    }
}

void init_brws_ports(Fl_Check_Browser *brws)
{
    for(int i = 1; i <= 65535; i++){
        char temp[6] = {};    
        IP::itoa(i, temp);
        brws->add(temp);
    }
}

void clbk_clean_all(Fl_Widget *, void *data)
{
    Fl_Check_Browser* brws = reinterpret_cast<Fl_Check_Browser*>(data);
    brws->check_none();
}

void clbk_select_all(Fl_Widget *, void *data)
{
    Fl_Check_Browser* brws = reinterpret_cast<Fl_Check_Browser*>(data);
    brws->check_all();
}

void clbk_btn_active_mode(Fl_Widget *, void *data)
{
    NetGuardUserInterface* n = reinterpret_cast<NetGuardUserInterface*>(data); 
    if(n->btn_active_mode->value()){
        n->schedule->TurnOnActiveMode();
    } else {
        n->schedule->TurnOffActiveMode();
    }
}

void clbk_stop_full_scan(Fl_Widget *, void *data)
{
    NetGuardUserInterface* n = reinterpret_cast<NetGuardUserInterface*>(data); 
    if(n->progress->value() > n->progress->minimum())
        n->schedule->manager.UsrStopFullScan();
}

void clbk_stop_port_scan(Fl_Widget *w, void *data)
{
    NetGuardUserInterface* n = reinterpret_cast<NetGuardUserInterface*>(data); 
    if(n->ports_scan_progress->value() > n->ports_scan_progress->minimum())
        n->schedule->manager.UsrStopPortScan();
}
