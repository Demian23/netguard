#include "../include/fd_handlers.h"

#include <unistd.h>
/********************************* FD_HANDLERS ******************************/

FdHandler::~FdHandler()
{
   if(own)
       close(fd);
}

void FdHandler::SetEvents(int a_events)
{
    events = a_events;
}

void FdHandler::ChangeFd(int a_fd, bool a_own, int a_events)
{
    close(fd);
    fd = a_fd;
    own = a_own;
    events = a_events;
}
