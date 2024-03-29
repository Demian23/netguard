#ifndef ERRORS_DEF
#define ERRORS_DEF
#include <errno.h>
namespace errors{
    int GetLogFd();
    void Quit(const char *fmt, ...);
    void Msg(const char *fmt, ...);
    void Dump(const char *fmt, ...);
    void Sys(const char *fmt, ...);
    void SysRet(const char *fmt, ...);
    void SetLogFile(const char*filename);
    void EndLogging();
};

#endif
