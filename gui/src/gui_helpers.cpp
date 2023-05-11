#include "gui_helper.h"
#include "../../srcs/include/host_addr.h"

#include "gui.h"

void init_interface_choices(Fl_Choice *choice)
{
    host_addr::interface_map interfaces = host_addr::get_net_interfaces();
    for(host_addr::interface_map::iterator it = interfaces.begin();
            it != interfaces.end(); it++){
        choice->add(it->first.c_str());
    }
}
void clbk_interface_choice(Fl_Widget* w, void* data)
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
        // show alert
    }
}
