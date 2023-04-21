#ifndef ERRORS_DEF
#define ERRORS_DEF

namespace errors{
    void Quit(const char *fmt, ...);
    void Msg(const char *fmt, ...);
    void Dump(const char *fmt, ...);
    void Sys(const char *fmt, ...);
    void SysRet(const char *fmt, ...);
};

#endif
