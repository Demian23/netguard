#ifndef EVSEL_DEFF
#define EVSEL_DEFF

#include "fd_handlers.h"
#include <poll.h>

// maybe connect FdHandler* and pollfd in some class that will controll its 
// condition equality; smth as lead pointers can be usefull here
// lets call it EventHandler, or even make it as container?

//EventContainer: remove Items with FdHandlers -1 when add new one
//maybe its better to make here cursor or smth ?
class EHCursor;
class EventSelector;
class EventHandler{
    friend class EHCursor;
    friend class EventSelector;
    typedef struct pollfd pfd;
    nfds_t nfds;
    pfd *fds;
    bool *timeouts;
    unsigned fds_size;
    enum{default_size = 64, free = -2, clue = 12};
    struct Item{
        FdHandler *h;
        unsigned index;
        Item *next;
        Item(FdHandler *new_h, unsigned new_index)
            : h(new_h), index(new_index), next(0) {} 
        ~Item(){if(h != 0) delete h;}
    };
    Item *p;
    unsigned FindIndex(FdHandler *h);
    unsigned FindFreeIndex();
    void ExpandFds();
    bool RemoveHandler(unsigned index);
    int EventsToInt(FdHandler *h);
    bool EventsToTimeout(FdHandler *h);
    void FreeResources(int index, pfd *arr, bool *t);
public:
    EventHandler() : nfds(0), fds(0), timeouts(0), fds_size(0), p(0){}
    bool AddHandler(FdHandler *new_h);
    EHCursor operator[](unsigned index);
    EHCursor operator[](FdHandler *h);
    ~EventHandler();
};

class EventSelector{
    EventHandler handlers; 
    bool end_run;
    EventSelector& operator=(const EventSelector&);
    EventSelector(const EventSelector&);
public:
    EventSelector() : handlers(), end_run(false){}
    ~EventSelector(){}
    bool Add(FdHandler *h);
    void Run(int timeout = -1);
    void UpdateEvents(FdHandler *h);
    void EndRun(){end_run = true;}
};
class EHCursor{
friend class EventHandler;
private:
    EventHandler& eh;
    EventHandler::Item *item;
    unsigned index;
    EHCursor(EventHandler &a_eh, EventHandler::Item *a_item)
        : eh(a_eh), item(a_item), index(item->index){}
public:
    operator FdHandler*(){return item == 0 ? 0 : item->h;}
    int operator&(int val);
    bool IsTimeout(){return eh.timeouts[index];}
    void ResetEvents();
    FdHandler* operator->(){return item->h;}
};


#endif
