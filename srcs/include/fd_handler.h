#ifndef FD_HANDLERS_DEFF
#define FD_HANDLERS_DEFF

#include <unistd.h>
class FdHandler{
public:
    enum Events{InEvent = 16, OutEvent = 32, ErrEvent = 64, None = 0, Timeout = 8};
protected:
    int fd;
    int events;
    short cond; 
    bool own;
    FdHandler(const FdHandler&);
    FdHandler& operator=(const FdHandler&);
public:
    enum{done = 16, work = 8};
    FdHandler(int a_fd, bool own_fd = true) : fd(a_fd), events(None), cond(work), own(own_fd){};
    virtual ~FdHandler(){if(own) close(fd);}
    virtual int HandleRead() = 0;
    virtual int HandleWrite() = 0;
    virtual int HandleError() = 0;
    virtual int HandleTimeout() = 0;
    void ChangeFd(int a_fd, bool a_own, int a_events){
        close(fd);fd = a_fd;own = a_own;events = a_events;
    }
    void SetEvents(int a_events){events = a_events;}
    int GetFd()const{return fd;}
    int GetEvents()const {return events;}
    short GetCond() const{return cond;}
    bool IsOwn()const {return own;}
    void ExplicitlyEnd(){cond = done;}
};

#endif
