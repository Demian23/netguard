#include "../../gui/src/gui.h"
#include "../../srcs/include/errors.h"
#include "../../srcs/include/mac.h"
#include "../../srcs/include/ip.h"
#include "../../srcs/include/host_addr.h"
#include "../../srcs/include/scheduler.h"
#include "../../srcs/include/port_scanner.h"
#include <pthread.h>

void* scan_thr_f(void*d){EventSelector* sel = reinterpret_cast<EventSelector*>(d); sel->StartSelecting();return 0;}

int main()
{
    errors::SetLogFile("netguard.log");
    EventSelector selector;
    NodesManager manager; 
    Scheduler* schedule = new Scheduler(selector, manager);
    pthread_t scan_thread;
    pthread_create(&scan_thread, 0, scan_thr_f, &selector); 
    schedule->SetThreadId(scan_thread);
    NetGuardUserInterface n(schedule);
    n.addLogFile();
    n.show();
    Fl::run();
    pthread_join(scan_thread, 0);
    return 0;
}
