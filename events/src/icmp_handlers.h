#ifndef ICMP_HANDLERS_DEF 
#define ICMP_HANDLERS_DEF

#include "fd_handlers.h"
#include "event_selector.h"
#include "../../net/src/icmp.h"

class SendEcho : public FdHandler{
private:
    enum{request_count = 1};
    EventSelector& sel;
    struct sockaddr_in dest_addr;
    int id;
    int seq;
    virtual int HandleRead();
    virtual int HandleWrite();
public:
    SendEcho(int a_fd, bool own, const char *ip, EventSelector& father, int a_id);
    virtual int HandleTimeout();
    virtual int HandleError();
    virtual ~SendEcho(){}
};

class RecvEcho: public FdHandler{
private:
    msghdr msg;
    int seq;
    int id;
    virtual int HandleWrite();
    virtual int HandleTimeout();
public:
    RecvEcho(int a_fd, bool own, int a_id);
    virtual int HandleError();
    virtual int HandleRead();
    bool Exists() const {return seq;}
    virtual ~RecvEcho(){} //wrong need to free at least buffers in msg

};

#endif
