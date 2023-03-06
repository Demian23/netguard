#ifndef FD_HANDLERS_DEFF
#define FD_HANDLERS_DEFF

class FdHandler{
public:
    enum Events{InEvent = 16, OutEvent = 32, ErrEvent = 64, None = 0, Timeout = 8, 
        IOEvent = InEvent + OutEvent, IEEvent = InEvent + ErrEvent, 
        OEEvent = OutEvent + ErrEvent, IOEEvent = OutEvent + InEvent + ErrEvent,
        IETEvent = InEvent + ErrEvent + Timeout,
        IOETEvent = OutEvent + ErrEvent + InEvent + Timeout};
protected:
    int fd;
    int events;
    short cond; 
    bool own;
    FdHandler(const FdHandler&);
    FdHandler& operator=(const FdHandler&);
public:
    enum{done = 10, work = 8};
    FdHandler(int a_fd, bool own_fd = true) : fd(a_fd), events(None), cond(work), own(own_fd){};
    virtual ~FdHandler();
    virtual int HandleRead() = 0;
    virtual int HandleWrite() = 0;
    virtual int HandleError() = 0;
    virtual int HandleTimeout() = 0;
    void SetEvents(int a_events);
    int GetFd()const{return fd;}
    int GetEvents()const {return events;}
    short GetCond() const{return cond;}
    void ExplicitlyEnd(){cond = done;}
};

class StreamHandler : public FdHandler{
protected:
    struct Buffer{
        char *stream_buffer;
        unsigned bs;
        unsigned b_cond;
        Buffer(unsigned size) : bs(size), b_cond(0){
           stream_buffer = new char[bs]; 
        }
        ~Buffer(){delete[] stream_buffer;}
    };
public:
    enum{error = -1, df_bs = 4096};
    StreamHandler(int a_fd, bool own_fd = true) : FdHandler(a_fd, own_fd){}
    virtual int HandleError();
};

class RStreamHandler: public StreamHandler{
private:
    virtual int HandleWrite();
protected:
    Buffer rbuff;
public:
    RStreamHandler(int a_fd, bool own_fd = true, unsigned buffer_size = df_bs)
        : StreamHandler(a_fd, own_fd), rbuff(buffer_size){}; 
    virtual ~RStreamHandler(){}
    Buffer* GetBuffer(){return &rbuff;}
    virtual int HandleRead();
};

class WStreamHandler : public StreamHandler{
private:
    virtual int HandleRead();
protected:
   Buffer *wbuff; 
public:
   WStreamHandler(int a_fd, bool own_fd = true, Buffer *a_buffer = 0)
       : StreamHandler(a_fd, own_fd), wbuff(a_buffer){}
   virtual int HandleWrite();
   virtual ~WStreamHandler(){}
};

#endif
