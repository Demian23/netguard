#ifndef SOCKET_HANDLERS_DEF
#define SOCKET_HANDLERS_DEF
#include "fd_handlers.h"
#include "event_selector.h"
#include "../../aux/src/dicmp.hpp"

class SendICMPHandler : public FdHandler{
private:
    enum{request_count = 1};
    EventSelector& sel;
    struct sockaddr_in dest_addr;
    int id;
    int seq;
    virtual int HandleRead();
    virtual int HandleWrite();
public:
    SendICMPHandler(int a_fd, bool own, const char *ip, EventSelector& father, int a_id);
    virtual int HandleTimeout();
    virtual int HandleError();
    virtual ~SendICMPHandler(){}
};

class RecvICMPHandler : public FdHandler{
private:
    struct msghdr msg;
    int seq;
    int id;
    virtual int HandleWrite();
    virtual int HandleTimeout();
public:
    RecvICMPHandler(int a_fd, bool own, int a_id);
    virtual int HandleError();
    virtual int HandleRead();
    bool Exists() const {return seq;}
    virtual ~RecvICMPHandler(){} //wrong need to free at least buffers in msg
};

#endif
