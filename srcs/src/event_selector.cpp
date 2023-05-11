#include <algorithm>
#include <errno.h>

#include "../include/event_selector.h"

void EventSelector::AddEvent(IEvent *e)
{
    //consistency
    int index = polling.add_fd(e->GetDescriptor(), e->ListeningEvents()); 
    if(index >= events.size())
        events.resize(index + 1);
    events[index] = e;
}
void EventSelector::EndSelecting(){end_selecting = true;}
void EventSelector::DeleteEvent(IEvent *e)
{
    int index = polling.delete_fd(e->GetDescriptor());
    delete events[index];
    events[index] = 0;
}
EventSelector::~EventSelector(){std::for_each(events.begin(), events.end(), [](IEvent* e){if(e)delete e;});}
void EventSelector::StartSelecting(int timeout)
{
    if(timeout < -1) return;
    end_selecting = false;
    int ret = 0;
    do{
        ret = poll(polling.fds, polling.current_size, timeout); 
        if(ret < 0){
            if(errno == EINTR || errno == EAGAIN)
                continue;
            else break;
        }
        for(int i = 0; i < polling.current_size; i++){
            if(polling.fds[i].revents & POLLERR)
                events[i]->OnError();
            if(polling.fds[i].revents & POLLNVAL)
                events[i]->OnError();
            if(polling.fds[i].revents & POLLIN)
                events[i]->OnRead();
            if(polling.fds[i].revents & POLLOUT)
                events[i]->OnRead();
            if(events[i] && events[i]->ListeningEvents() & Events::Timeout && ret == 0)
                events[i]->OnTimeout();
            if(events[i] && events[i]->ListeningEvents() & Events::Any)
                events[i]->OnAnyEvent();
        }
        std::for_each(events.begin(), events.end(), [this](IEvent* e)
            {if(e && e->End()) DeleteEvent(e);});
    }while(!end_selecting);
}

short EventsToPollEvents(short events)
{
    short res = 0;
    res |= events & Read ? POLLIN : 0;
    res |= events & Write ? POLLOUT : 0;
    res |= events & Error ? POLLERR: 0;
    return res;
}

poll_arr::poll_arr() 
    : fds(0), current_size(0), real_size(start_size)
{
    expand();
}

int poll_arr::add_fd(int fd, short events)
{
    int i;
    for(i = 0; i < real_size; i++)
        if(fds[i].fd == empty)
            break;
    if(i == real_size){
        expand();
    }
    current_size++;
    fds[i].fd = fd;
    fds[i].events = EventsToPollEvents(events);
    fds[i].revents = 0;
    return i;
}

void poll_arr::expand()
{
    real_size *= 2;
    pollfd* new_fds = new pollfd[real_size];
    int i = 0;
    for(; i < current_size; i++){
        new_fds[i] = fds[i];
    }
    for(; i < real_size; i++){
        new_fds[i] = {empty, 0, 0};
    }
    delete[] fds;
    fds = new_fds;
}

int poll_arr::delete_fd(int fd)
{
   int i; 
   for(i = 0; i < real_size; i++)
       if(fds[i].fd == fd){
            fds[i] = {empty, 0, 0};
            break;
       }
   if(i == current_size - 1)
       current_size--;
   return i;
}
