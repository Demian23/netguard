#ifndef ARP_HANDLER_DEF
#define ARP_HANDLER_DEF

#include "fd_handlers.h"
#include "event_selector.h"
#include "../../net/src/arp.h"
#include <string>

class ARPHandler : public FdHandler{
private:
    EventSelector &sel;
    ARP::arp_pair pair;
    sockaddr_in ip;
    ether_addr mac;
    std::string interface;
    std::string dest_ip;
    char* bpf_buffer; 
    int buffer_length;
    bool find;
public:
    ARPHandler(sockaddr_in a_ip, ether_addr a_mac, const std::string& dest, 
        const std::string& a_interface, EventSelector &a_sel);
    virtual ~ARPHandler();
    virtual int HandleRead();    
    virtual int HandleError();
    virtual int HandleWrite();
    virtual int HandleTimeout();
    const ARP::arp_pair GetPair() const{return pair;}
    bool Find()const{return find;}
};
#endif
