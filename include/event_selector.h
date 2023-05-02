#ifndef EVSEL_DEFF
#define EVSEL_DEFF

#include "fd_handlers.h"
#include <poll.h>

class EHCursor;
class EventSelector;
class EventHandler{
    friend class EHCursor;
    friend class EventSelector;
    nfds_t nfds;
    pollfd *fds;
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
    void RemoveFreeHandlers();
    int EventsToInt(FdHandler *h);
    bool EventsToTimeout(FdHandler *h);
    void FreeResources(int index, pollfd *arr, bool *t);
public:
    EventHandler() : nfds(0), fds(0), timeouts(0), fds_size(0), p(0){ExpandFds();}
    bool AddHandler(FdHandler *new_h);
    EHCursor operator[](unsigned index);
    EHCursor operator[](FdHandler *h);
    unsigned Size() const {return nfds;}
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
    EventHandler* GetHandlers(){return &handlers;}
};

class EHCursor{
friend class EventHandler;
private:
    EventHandler& eh;
    EventHandler::Item *item;
    unsigned index;
    EHCursor(EventHandler &a_eh, EventHandler::Item *a_item)
        : eh(a_eh), item(a_item), index(a_item->index){}
    EHCursor(EventHandler& a_eh)
        : eh(a_eh), item(0), index(-1){}
public:
    operator FdHandler*(){return item == 0 ? 0 : item->h;}
    int operator&(int val);
    bool IsTimeout(){if(-1 != index) return eh.timeouts[index]; else return false;}
    void ResetEvents();
    void ResetFd(int a_fd, bool a_own, int a_events);
    FdHandler* operator->(){return item == 0 ? 0 : item->h;}
};


#endif
