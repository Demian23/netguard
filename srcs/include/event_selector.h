#ifndef EVSEL_DEFF
#define EVSEL_DEFF

#include <sys/poll.h>
#include <vector>
#include "event.h"

struct poll_arr{
    enum{start_size = 16, empty = -1};
    pollfd* fds;
    int real_size;
    poll_arr();
    int add_fd(int fd, short events);
    int delete_fd(int fd);
    void expand();
    nfds_t get_current_size();
    ~poll_arr(){delete[] fds;}
};

class EventSelector final{
public:
    EventSelector() : timeout(-1), end_selecting(false){}
    void AddEvent(IEvent* e);
    void DeleteEvent(IEvent* e);
    void StartSelecting();
    void EndSelecting();
    void SetTimeout(int t);
    int GetTimeout()const;
    ~EventSelector();
private:
    std::vector<IEvent*> events;
    poll_arr polling;
    int timeout;
    bool end_selecting;
};

#endif
