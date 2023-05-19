#ifndef EVENT_DEF
#define EVENT_DEF
enum Events{Read = 1, Write = 2, Error = 4, Timeout = 8, Any = 16, None = 32};
class IEvent{
public:
    virtual void OnRead(){}; 
    virtual void OnWrite(){}; 
    virtual void OnError(){}; 
    virtual void OnTimeout(){}; 
    virtual void OnAnyEvent(){};
    virtual short ListeningEvents() const = 0; 
    virtual void ResetEvents(int events){};
    virtual int GetDescriptor()const = 0;
    virtual bool End() const = 0 ;
    virtual ~IEvent(){};
};
#endif // !EVENT_DEF
