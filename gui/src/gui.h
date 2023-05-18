// generated by Fast Light User Interface Designer (fluid) version 1.0308

#ifndef gui_h
#define gui_h
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Text_Display.H>

#include "../../srcs/include/scheduler.h"

class NetGuardUserInterface {
public:
  NetGuardUserInterface(Scheduler* a_sched);
  Fl_Double_Window *main_window;
  Fl_Tabs *tbs_main;
  Fl_Group *grp_nodes;
  Fl_Browser *brws_nodes;
  Fl_Group *grp_node_info;
  Fl_Output *out_name;
  Fl_Output *out_ip;
  Fl_Output *out_mac;
  Fl_Output *out_vendor;
  Fl_Button *btn_ports_scan;
  Fl_Check_Browser *brws_ports;
  Fl_Progress *ports_scan_progress;
  Fl_Output *out_type;
  Fl_Browser *brws_opened_ports;
  Fl_Button *btn_clean_ports_brws;
  Fl_Button *btn_choos_all_ports;
  Fl_Browser *brws_filtered_ports;
  Fl_Browser *brws_closed_ports;
  Fl_Group *grp_settings;
  Fl_Choice *choice_interface;
  Fl_Output *out_own_ip;
  Fl_Output *out_own_mask;
  Fl_Button *btn_scan;
  Fl_Progress *progress;
  Fl_Output *out_own_mac;
  Fl_Input *first_ip;
  Fl_Input *last_ip;
  Fl_Output *out_net;
  Fl_Light_Button *btn_active_mode;
  Fl_Text_Display *log_display;
  Scheduler* schedule;
  void show();
  void updateNodesBrowser();
  void updatePortsBrowser(const std::string& destination);
};
#endif
