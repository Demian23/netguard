#include "../include/errors.h"

#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>

namespace errors{
enum{max_len = 1024};
FILE* log_file = 0;
void SetLogFile(const char *filename = 0)
{
    if(filename)
        log_file = fopen(filename, "w"); 
    else log_file = stderr;
}
void EndLogging(){fclose(log_file);}

void print_err(FILE* out, bool errno_flag, const char *fmt, va_list ap)
{
   int errno_save, size; 
   char buff[max_len + 1] = {};
   errno_save = errno;
   time_t now = time(0);
   struct tm* real_time;
   real_time = localtime(&now);
   size = strftime(buff, max_len, "%Y-%m-%d.%X ", real_time);
   size += vsnprintf(buff + size, max_len, fmt, ap);
   if(errno_flag){
       snprintf(buff + size, max_len - size, ": %s\n", strerror(errno_save));
   } else {
       buff[size] = '\n';
   }
   fputs(buff, out);
   fflush(out);
}

void Quit(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(log_file, false, fmt, ap);
    va_end(ap);
    exit(1);
}

void Msg(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(log_file, false, fmt, ap);
    va_end(ap);
}

void Dump(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(log_file, true, fmt, ap);
    va_end(ap);
    abort();
    exit(1);
}

void Sys(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(log_file, true, fmt, ap);
    va_end(ap);
    exit(1);
}

void SysRet(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print_err(log_file, true, fmt, ap);
    va_end(ap);
}

}
