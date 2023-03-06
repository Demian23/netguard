#ifndef NETWORK_ERROR_DEF
#define NETWORK_ERROR_DEF
#include <errno.h>
namespace DERR{
    void Quit(const char *fmt, ...);
    void Msg(const char *fmt, ...);
    void Dump(const char *fmt, ...);
    void Sys(const char *fmt, ...);
    void SysRet(const char *fmt, ...);
};


#endif
