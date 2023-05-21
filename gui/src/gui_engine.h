#ifndef GUI_CALLBACKS_DEF
#define GUI_CALLBACKS_DEF
#include <FL/Fl.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Table_Row.H>
#include "../../srcs/include/nodes_manager.h"
void init_interface_choices(Fl_Choice* choice);
void init_brws_ports(Fl_Check_Browser* brws);
void clbk_choice_interface(Fl_Widget* w, void* data);
void clbk_full_scan(Fl_Widget*w, void* data);
void clbk_main_window(Fl_Widget* w, void* data);
void clbk_nodes_brws(Fl_Widget* w, void* data);
void clbk_port_scan(Fl_Widget* w, void *data);
void clbk_select_all(Fl_Widget*, void* data);
void clbk_clean_all(Fl_Widget*, void* data);
void clbk_btn_active_mode(Fl_Widget*, void* data);
void clbk_stop_port_scan(Fl_Widget*, void* data);
void clbk_stop_full_scan(Fl_Widget*, void* data);


class PortsTable : public Fl_Table_Row{
    NodesManager& manager;
    std::vector<uint16_t> printable_ports;
    std::string current_ip;
    void DrawHeader(const char *str, int X, int Y, int W, int H);
    void DrawCell(const char *str, int X, int Y, int W, int H, bool select);
    void draw_cell(TableContext context, int R=0, int C=0, int X=0, int Y=0, int W=0, int H=0);
    void DrawCellContent(int R, int C, int X, int Y, int W, int H);
public:
    PortsTable(int x, int y,int table_w, int table_h, const char *label, NodesManager& m);
    void UpdateTable(const std::string& ip, std::vector<uint16_t> ports){
        current_ip = ip; printable_ports = ports;
        redraw();
    }
};


#endif // !GUI_CALLBACKS_DEF
