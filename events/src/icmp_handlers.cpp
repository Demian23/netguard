#include "icmp_handlers.h"
#include "../../net/src/errors.h"
#include "../../net/src/host_addr.h"
#include "fd_handlers.h"


SendEcho::SendEcho(int a_fd, bool own, const char *ip, EventSelector& father,
    int a_id)
    : FdHandler(a_fd, own), sel(father), id(a_id), seq(0)
{
    dest_addr = host_addr::set_addr(ip, AF_INET);
}

int SendEcho::HandleRead()
{
    errors::Msg("Try to read from SendEcho");
    return -1;
}

int SendEcho::HandleError()
{
    errors::SysRet("Some error on SendEcho");
    return 0;
}

int SendEcho::HandleWrite()
{
    errors::Msg("Try to write from SendEcho");
    return -1;
}

int SendEcho::HandleTimeout()
{
    if(seq < request_count)
        ICMP::send_echo(fd, id, seq++, &dest_addr, sizeof(dest_addr));
    else 
        sel.EndRun();
    return 0;
}


RecvEcho::RecvEcho(int a_fd, bool own, int a_id)
    : FdHandler(a_fd, own), seq(0), id(a_id)
{
   memset(&msg, 0, sizeof(msg)); 
}

int RecvEcho::HandleError()
{
    errors::SysRet("Some errror on RecvEcho");  
    return 0;
}

int RecvEcho::HandleRead()
{
    ssize_t len = ICMP::recv_reply(fd , &msg);
    if(ICMP::process_reply(fd, reinterpret_cast<char *>(msg.msg_iov->iov_base), len, id))
        seq++;
    return 0;
}

int RecvEcho::HandleWrite()
{
    errors::Msg("Try to write form RecvEcho");
    return -1;
}

int RecvEcho::HandleTimeout()
{
    errors::Msg("Try to handle timeout form RecvEcho");
    return -1;
}

