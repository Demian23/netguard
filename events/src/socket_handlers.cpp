#include "socket_handlers.hpp"
#include "../../aux/src/dneterr.hpp"
#include "fd_handlers.h"
#include <unistd.h>
SendICMPHandler::SendICMPHandler(int a_fd, bool own, const char *ip, EventSelector& father,
    int a_id)
    : FdHandler(a_fd, own), sel(father), id(a_id), seq(0)
{
    dest_addr = DNET::set_addr(ip, AF_INET);
}

int SendICMPHandler::HandleRead()
{
    DERR::Msg("Try to read from SendICMPHandler");
    return -1;
}

int SendICMPHandler::HandleError()
{
    DERR::SysRet("Some error on SendICMPHandler");
    return 0;
}

int SendICMPHandler::HandleWrite()
{
    DERR::Msg("Try to write from SendICMPHandler");
    return -1;
}

int SendICMPHandler::HandleTimeout()
{
    if(seq < request_count)
        DICMP::send_echo(fd, id, seq++, &dest_addr, sizeof(dest_addr));
    else 
        sel.EndRun();
    return 0;
}


RecvICMPHandler::RecvICMPHandler(int a_fd, bool own, int a_id)
    : FdHandler(a_fd, own), seq(0), id(a_id)
{
   memset(&msg, 0, sizeof(msg)); 
}

int RecvICMPHandler::HandleError()
{
    DERR::SysRet("Some errror on RecvICMPHandler");  
    return 0;
}

int RecvICMPHandler::HandleRead()
{
    ssize_t len = DICMP::recv_reply(fd , &msg);
    if(DICMP::process_reply(fd, reinterpret_cast<char *>(msg.msg_iov->iov_base), len, id))
        seq++;
    return 0;
}

int RecvICMPHandler::HandleWrite()
{
    DERR::Msg("Try to write form RecvICMPHandler");
    return -1;
}

int RecvICMPHandler::HandleTimeout()
{
    DERR::Msg("Try to handle timeout form RecvICMPHandler");
    return -1;
}
