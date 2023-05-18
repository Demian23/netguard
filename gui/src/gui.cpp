// generated by Fast Light User Interface Designer (fluid) version 1.0308

#include "gui.h"
#include "gui_engine.h"
#include "../../srcs/include/ip.h"
#include "../../srcs/include/port_scanner.h"
#include <cstdio>
#include <string>

NetGuardUserInterface::NetGuardUserInterface(Scheduler* a_sched) : schedule(a_sched) {
  { main_window = new Fl_Double_Window(1045, 791, "NetGuard");
    main_window->color(FL_DARK1);
    main_window->labelcolor((Fl_Color)1);
    main_window->callback(clbk_main_window, this);
    { tbs_main = new Fl_Tabs(15, 15, 1005, 750);
      tbs_main->labelsize(18);
      { grp_nodes = new Fl_Group(15, 40, 1000, 725, "Nodes");
        grp_nodes->labelsize(18);
        { brws_nodes = new Fl_Browser(40, 70, 210, 655, "Nodes list");
          brws_nodes->type(2);
          brws_nodes->labelsize(18);
          brws_nodes->textsize(18);
          brws_nodes->callback(clbk_nodes_brws, this);
        } // Fl_Browser* brws_nodes
        { grp_node_info = new Fl_Group(290, 70, 695, 655, "Node info");
          grp_node_info->box(FL_UP_BOX);
          grp_node_info->color((Fl_Color)42);
          grp_node_info->labelsize(18);
          grp_node_info->align(Fl_Align(FL_ALIGN_BOTTOM));
          { out_name = new Fl_Output(325, 100, 170, 40, "name");
            out_name->labelsize(18);
            out_name->textsize(18);
            out_name->align(Fl_Align(FL_ALIGN_TOP_LEFT));
          } // Fl_Output* out_name
          { out_ip = new Fl_Output(325, 160, 170, 40, "ip");
            out_ip->labelsize(18);
            out_ip->textsize(18);
            out_ip->align(Fl_Align(FL_ALIGN_TOP_LEFT));
          } // Fl_Output* out_ip
          { out_mac = new Fl_Output(325, 220, 170, 40, "mac");
            out_mac->labelsize(18);
            out_mac->textsize(18);
            out_mac->align(Fl_Align(FL_ALIGN_TOP_LEFT));
          } // Fl_Output* out_mac
          { out_vendor = new Fl_Output(325, 280, 170, 40, "vendor");
            out_vendor->labelsize(18);
            out_vendor->textsize(18);
            out_vendor->align(Fl_Align(FL_ALIGN_TOP_LEFT));
          } // Fl_Output* out_vendor
          { btn_ports_scan = new Fl_Button(790, 665, 100, 35, "scan ports");
            btn_ports_scan->labelsize(18);
            btn_ports_scan->callback(clbk_port_scan, this);
          } // Fl_Button* btn_ports_scan
          { brws_ports = new Fl_Check_Browser(325, 400, 170, 240, "choose ports to scan");
            brws_ports->type(2);
            brws_ports->labelsize(18);
            brws_ports->textsize(18);
            init_brws_ports(brws_ports);
          } // Fl_Check_Browser* brws_ports
          { ports_scan_progress = new Fl_Progress(716, 611, 245, 30, "scanning");
            ports_scan_progress->labelsize(18);
          } // Fl_Progress* ports_scan_progress
          { out_type = new Fl_Output(325, 340, 170, 40, "type");
            out_type->labelsize(18);
            out_type->textsize(18);
            out_type->align(Fl_Align(FL_ALIGN_TOP_LEFT));
          } // Fl_Output* out_type
          { brws_scanned_ports = new Fl_Browser(725, 105, 230, 475, "ports");
            brws_scanned_ports->labelsize(18);
            brws_scanned_ports->textsize(18);
          } // Fl_Browser* brws_scanned_ports
          grp_node_info->end();
        } // Fl_Group* grp_node_info
        grp_nodes->end();
      } // Fl_Group* grp_nodes
      { grp_settings = new Fl_Group(20, 50, 995, 710, "Settings");
        grp_settings->labelsize(18);
        grp_settings->hide();
        { choice_interface = new Fl_Choice(100, 70, 120, 30, "interface");
          choice_interface->down_box(FL_BORDER_BOX);
          choice_interface->labelsize(18);
          choice_interface->textsize(18);
          choice_interface->callback(clbk_choice_interface, this);
          init_interface_choices(choice_interface);
        } // Fl_Choice* choice_interface
        { out_own_ip = new Fl_Output(80, 130, 160, 30, "ip");
          out_own_ip->labelsize(18);
          out_own_ip->textsize(18);
        } // Fl_Output* out_own_ip
        { out_own_mask = new Fl_Output(80, 180, 160, 30, "mask");
          out_own_mask->labelsize(18);
          out_own_mask->textsize(18);
        } // Fl_Output* out_own_mask
        { btn_scan = new Fl_Button(190, 450, 155, 40, "full scan");
          btn_scan->labelsize(18);
          btn_scan->callback(clbk_full_scan, this);
        } // Fl_Button* btn_scan
        { progress = new Fl_Progress(80, 400, 380, 30, "scan progress");
          progress->labelsize(18);
        } // Fl_Progress* progress
        { out_own_mac = new Fl_Output(80, 230, 160, 30, "mac");
          out_own_mac->labelsize(18);
          out_own_mac->textsize(18);
        } // Fl_Output* out_own_mac
        { first_ip = new Fl_Input(80, 340, 160, 30, "first");
          first_ip->labelsize(18);
          first_ip->textsize(18);
        } // Fl_Input* first_ip
        { last_ip = new Fl_Input(300, 340, 160, 30, "last");
          last_ip->labelsize(18);
          last_ip->textsize(18);
        } // Fl_Input* last_ip
        { out_net = new Fl_Output(80, 280, 160, 30, "net");
          out_net->labelsize(18);
          out_net->textsize(18);
        } // Fl_Output* out_net
        grp_settings->end();
      } // Fl_Group* grp_settings
      tbs_main->end();
    } // Fl_Tabs* tbs_main
    main_window->size_range(1030, 700, 1030, 700);
    main_window->end();
  } // Fl_Double_Window* main_window
}

void NetGuardUserInterface::show(){main_window->show();}

void NetGuardUserInterface::updateNodesBrowser()
{
    const NetMap& map = schedule->manager.GetMap();
    brws_nodes->clear();
    int i = 0;
    for(NetMap::const_iterator it = map.begin(); it != map.end(); it++, i++){
        if(it->second.is_active)
            brws_nodes->insert(i, (it->first + " (on)").c_str(), (void*)&it->second);
        else
            brws_nodes->insert(i, (it->first + " (off)").c_str(), (void*)&it->second);
    }
}

void NetGuardUserInterface::updatePortsBrowser(const std::string& destination)
{
    brws_scanned_ports->clear();
    const ports_storage& temp = 
        schedule->manager.GetMap()[destination].ports;
    for(ports_storage::const_iterator it = temp.begin(); 
            it != temp.end(); it++){
        char buffer[64] = {};
        sprintf(buffer, "tcp/%d %s", it->first, ports_conditions[it->second]);
        brws_scanned_ports->add(buffer);
    }
}
