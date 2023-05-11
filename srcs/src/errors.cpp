#include "../include/errors.h"

#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

namespace errors{
    enum{max_len = 1024};

void print_err(FILE* out, bool errno_flag, const char *fmt, va_list ap)
{
   int errno_save, size; 
   char buff[max_len + 1] = {};

   errno_save = errno;
   vsnprintf(buff, max_len, fmt, ap);
   size = strlen(buff);
   if(errno_flag){
       snprintf(buff + size, max_len - size, ": %s\n", strerror(errno_save));
   } else {
       buff[size] = '\n';
   }
   fputs(buff, out);
   fflush(out);
}

void print_err(int out, bool errno_flag, const char *fmt, va_list ap)
{
   int errno_save, size; 
   char buff[max_len + 1] = {};

   errno_save = errno;
   vsnprintf(buff, max_len, fmt, ap);
   size = strlen(buff);
   if(errno_flag){
       snprintf(buff + size, max_len - size, ": %s\n", strerror(errno_save));
   } else {
       buff[size] = '\n';
   }
   write(out, buff, size);
}

void Quit(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(stderr, false, fmt, ap);
    va_end(ap);
    exit(1);
}

void Msg(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(stderr, false, fmt, ap);
    va_end(ap);
}

void Dump(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(stderr, true, fmt, ap);
    va_end(ap);
    abort();
    exit(1);
}

void Sys(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(stderr, true, fmt, ap);
    va_end(ap);
    exit(1);
}

void SysRet(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(stderr, true, fmt, ap);
    va_end(ap);
}

void WriteLog(int log_file, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(log_file, false, fmt, ap);
    va_end(ap);
}

void WriteErrorLog(int log_file, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(log_file, true, fmt, ap);
    va_end(ap);
}

}
