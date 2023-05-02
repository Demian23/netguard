#include "../include/event_selector.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

//      EventHandler

EHCursor EventHandler::operator[](FdHandler *h)
{
    return (*this)[FindIndex(h)];
}

unsigned EventHandler::FindIndex(FdHandler *h)
{
    int i;
    for(i = 0; i < nfds; i++)
        if(fds[i].fd == h->GetFd())
            break;
    return i == nfds ? -1 : i;
}

void EventHandler::ExpandFds()
{
    unsigned new_size = fds_size > 0 ? fds_size << 1 : default_size;
    pollfd *new_fds = new pollfd[new_size];
    bool *new_timeouts = new bool[new_size];
    int i;
    for(i = 0; i < fds_size; i++){
        new_fds[i].fd = fds[i].fd;
        new_fds[i].events = fds[i].events;
        new_fds[i].revents = fds[i].revents;
        new_timeouts[i] = timeouts[i];
    }
    for(int j = i; j < new_size; j++){
        FreeResources(j, new_fds, new_timeouts);
    }
    delete[] fds;
    delete[] timeouts;
    fds_size = new_size;
    fds = new_fds;
    timeouts = new_timeouts;
}

void EventHandler::RemoveFreeHandlers()
{
    Item *temp_p = p;
    while(temp_p != 0){
        if(temp_p->h->GetCond() == temp_p->h->done){
            Item* save_p = temp_p;
            temp_p = temp_p->next;
            RemoveHandler(save_p->index);
        } else 
            temp_p = temp_p->next;

    }
}

unsigned EventHandler::FindFreeIndex()
{
    for(int i = 0; i < fds_size; i++){
       if(fds[i].fd == free) 
           return i;
    }
    unsigned res = fds_size;
    ExpandFds();
    return res;
}

int EventHandler::EventsToInt(FdHandler *h)
{
    int result = 0;
    result |= h->GetEvents() & h->InEvent ? POLLIN : 0;
    result |= h->GetEvents() & h->OutEvent ? POLLOUT : 0;
    result |= h->GetEvents() & h->ErrEvent ? POLLERR : 0;
    return result;
}

bool EventHandler::EventsToTimeout(FdHandler *h)
{
    return h->GetEvents() & h->Timeout ? true : false;
}

bool EventHandler::AddHandler(FdHandler *new_h)
{
    RemoveFreeHandlers();
    if(new_h == 0)
        return false;
    if(p == 0){
        p = new Item(new_h, 0); 
        fds[0].fd = new_h->GetFd();
        fds[0].events = EventsToInt(new_h);
        fds[0].revents = 0;
        timeouts[0] = EventsToTimeout(new_h);
        nfds = 1;
    } else {
        unsigned index = FindFreeIndex();
        Item *new_item = new Item(new_h, index); 
        new_item->next = p;
        p = new_item;
        fds[index].fd = new_h->GetFd();
        fds[index].events = EventsToInt(new_h);
        fds[index].revents = 0;
        timeouts[index] = EventsToTimeout(new_h);
        nfds++;
    }    
    return true;
}

EHCursor EventHandler::operator[](unsigned int index)
{
    Item *temp_p;
    for(temp_p = p; temp_p != 0 && temp_p->index != index; temp_p = temp_p->next);
    if(temp_p)
        return EHCursor(*this, temp_p);
    else 
        return EHCursor(*this);
}
void EventHandler::FreeResources(int index, pollfd *arr, bool *t)
{
    arr[index].fd = free;
    arr[index].events = 0;
    arr[index].revents = 0;
    t[index] = false;
}
bool EventHandler::RemoveHandler(unsigned int index)
{
   Item *temp_p = p, *prev = 0; 
   if(temp_p == 0)
       return false;
   if(temp_p->index == index){
        nfds--;
        p = p->next;
        FreeResources(index, fds, timeouts);
        delete temp_p;
        return true;
   }
   for(prev = temp_p, temp_p = temp_p->next; temp_p != 0 && 
           temp_p->index != index; prev = temp_p, temp_p = temp_p->next){}
    if(temp_p == 0)
        return false;
    nfds--;
    prev->next = temp_p->next;
    FreeResources(index, fds, timeouts);
    delete temp_p;
    return true;
}

EventHandler::~EventHandler()
{
    if(fds != 0)
        delete[] fds;
    Item *temp_p;
    while(p != 0){
        temp_p = p; 
        p = p->next;
        delete temp_p;
    }
}

//          EHCursor

int EHCursor::operator&(int val)
{
    if(item == 0)
        return 0;
    return eh.fds[index].revents & val;
}

void EHCursor::ResetEvents()
{
    if(-1 != index){
        eh.fds[index].events = eh.EventsToInt(item->h); 
        eh.timeouts[index] = eh.EventsToTimeout(item->h);
        eh.fds[index].revents = 0;
    }
}

void EHCursor::ResetFd(int a_fd, bool a_own, int a_events)
{
    if(-1 != index){
        item->h->ChangeFd(a_fd, a_own, a_events);
        ResetEvents();
        eh.fds[index].fd = a_fd;
    }
}


//          EventSelector

void EventSelector::UpdateEvents(FdHandler *h)
{
    handlers[h].ResetEvents();
}

void EventSelector::Run(int timeout)
{
    int ret;
    if(timeout < -1) return;
    end_run = false;
    do{
        ret = poll(handlers.fds, handlers.nfds, timeout);
        if(ret < 0){
            if(errno == EINTR || errno == EAGAIN)
                continue;
            else
                break;
        }
        for(int i = 0; i < handlers.nfds; i++){
            if(handlers[i] & POLLIN)
                handlers[i]->HandleRead();
            if(handlers[i] & POLLOUT)
                handlers[i]->HandleWrite();
            if((handlers[i] & POLLERR) || (handlers[i] & POLLNVAL))
                handlers[i]->HandleError();
            if((ret == 0) && handlers[i].IsTimeout())
                handlers[i]->HandleTimeout();
        }
        handlers.RemoveFreeHandlers();
    }while(!end_run);
}

bool EventSelector::Add(FdHandler *h)
{
   return handlers.AddHandler(h); 
}
