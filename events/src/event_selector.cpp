#include "event_selector.h"
#include "fd_handlers.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/poll.h>
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
    pfd *new_fds = new pfd[new_size];
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

unsigned EventHandler::FindFreeIndex()
{
    Item *temp_p = p;
    while(temp_p != 0){
        if(temp_p->h->GetCond() == temp_p->h->done){
            RemoveHandler(temp_p->index);
        } 
        temp_p = temp_p->next;
    }
    
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
    if(new_h == 0)
        return false;
    if(p == 0){
        p = new Item(new_h, 0); 
        ExpandFds();
        nfds = 1;
        fds[0].fd = new_h->GetFd();
        fds[0].events = EventsToInt(new_h);
        fds[0].revents = 0;
        timeouts[0] = EventsToTimeout(new_h);
    } else {
        unsigned index = FindFreeIndex();
        Item *new_item = new Item(new_h, index); 
        new_item->next = p;
        p = new_item;
        nfds++;
        fds[index].fd = new_h->GetFd();
        fds[index].events = EventsToInt(new_h);
        fds[index].revents = 0;
        timeouts[index] = EventsToTimeout(new_h);
    }    
    return true;
}

EHCursor EventHandler::operator[](unsigned int index)
{
    Item *temp_p;
    for(temp_p = p; temp_p != 0 && temp_p->index != index; temp_p = temp_p->next);
    return EHCursor(*this, temp_p);
}
void EventHandler::FreeResources(int index, pfd *arr, bool *t)
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
        p = p->next;
        FreeResources(index, fds, timeouts);
        nfds--;
        delete temp_p;
        return true;
   }
   for(prev = temp_p, temp_p = temp_p->next; temp_p != 0 && 
           temp_p->index != index; prev = temp_p, temp_p = temp_p->next){}
    if(temp_p == 0)
        return false;
    prev->next = temp_p->next;
    FreeResources(index, fds, timeouts);
    nfds--;
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
   eh.fds[index].events = eh.EventsToInt(item->h); 
   eh.fds[index].revents = 0;
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
    bool timeout_flag = timeout != 0 && timeout != -1;
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
            if(timeout_flag & handlers[i].IsTimeout())
                handlers[i]->HandleTimeout();
        }
    }while(!end_run);
}

bool EventSelector::Add(FdHandler *h)
{
   return handlers.AddHandler(h); 
}
