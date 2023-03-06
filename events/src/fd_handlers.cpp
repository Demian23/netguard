#include "fd_handlers.h"

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

//          StreamHandler
int StreamHandler::HandleError()
{
    close(fd);
    own = false;
    cond = done;
    return cond;
}

//      RStreamHandler

int RStreamHandler::HandleRead()
{
    if(rbuff.b_cond != 0)
        return 0;
    int res = read(fd, rbuff.stream_buffer, rbuff.bs); 
    if(res == -1){
        rbuff.b_cond = error;
        return error;
    }
    rbuff.b_cond +=  res;
    return res;
}

int RStreamHandler::HandleWrite(){
    return error;
}


//      WStreamHandler

int WStreamHandler::HandleWrite()
{
    if(wbuff->b_cond == error)
        return 0;
    int res = write(fd, wbuff->stream_buffer, wbuff->b_cond);
    if(res == -1)
        return error;
    wbuff->b_cond -= res;
    return res;
}

int WStreamHandler::HandleRead(){
    return error;
}
