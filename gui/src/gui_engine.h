#ifndef GUI_CALLBACKS_DEF
#define GUI_CALLBACKS_DEF
#include <FL/Fl.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_ask.H>
void init_interface_choices(Fl_Choice* choice);
void clbk_choice_interface(Fl_Widget* w, void* data);
void clbk_full_scan(Fl_Widget*w, void* data);
void clbk_main_window(Fl_Widget* w, void* data);
void clbk_nodes_brws(Fl_Widget* w, void* data);
void clbk_port_scan(Fl_Widget* w, void *data);

#endif // !GUI_CALLBACKS_DEF
