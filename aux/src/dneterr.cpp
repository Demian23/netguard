#include "dneterr.hpp"

#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace DERR{
    enum{max_len = 1024};
    bool daemon_proc;
    void print_err(bool errno_flag, int level, const char *fmt, va_list ap);
}

void DERR::print_err(bool errno_flag, int level, const char *fmt, va_list ap)
{
   int errno_save, size; 
   char buff[max_len + 1];

   errno_save = errno;
   vsnprintf(buff, max_len, fmt, ap);
   size = strlen(buff);
   if(errno_flag){
       snprintf(buff + size, max_len - size, ": %s\n", strerror(errno_save));
   } else {
       buff[size] = '\n';
   }

   if(daemon_proc)
       syslog(level,"%s", buff);
   else{
       fflush(stdout);
       fputs(buff, stderr);
       fflush(stderr);
   }
}

void DERR::Quit(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(false, LOG_ERR, fmt, ap);
    va_end(ap);
    exit(1);
}

void DERR::Msg(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(false, LOG_INFO, fmt, ap);
    va_end(ap);
}

void DERR::Dump(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(true, LOG_ERR, fmt, ap);
    va_end(ap);
    abort();
    exit(1);
}

void DERR::Sys(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(true, LOG_ERR, fmt, ap);
    va_end(ap);
    exit(1);
}

void DERR::SysRet(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(true, LOG_ERR, fmt, ap);
    va_end(ap);
}
