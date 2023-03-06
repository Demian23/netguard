#ifndef ARP_HANDLER_DEF
#define ARP_HANDLER_DEF
#include "fd_handlers.h"
#include "event_selector.h"
#include "../../aux/src/darp.hpp"
#include "../../aux/src/dneterr.hpp"

class ARPHandler : public FdHandler{
private:
    enum{ArpTimeout=DARP::arp_timeout};
    EventSelector &sel;
    DARP::arp_pair *pairs;
    sockaddr_in ip;
    ether_addr mac;
    const char *interface;
    char **dest_ips;
    char *bpf_buffer; 
    const int pairs_size;
    int buffer_length;
    int timeout_counter;
public:
    ARPHandler(sockaddr_in a_ip, ether_addr a_mac, char **ips, const int len, 
        const char *a_interface, EventSelector &a_sel);
    virtual ~ARPHandler();
    virtual int HandleRead();    
    virtual int HandleError();
    virtual int HandleWrite();
    virtual int HandleTimeout();
    const DARP::arp_pair *GetPairs() const{return pairs;}
};
#endif
