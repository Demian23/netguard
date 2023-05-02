#include "../include/arper.h"

#include "../include/fd_handlers.h"
#include "../include/arp.h"
#include "../include/errors.h"

class ARPHandler : public FdHandler{
private:
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
        const std::string& a_interface);
    virtual ~ARPHandler();
    virtual int HandleRead();    
    virtual int HandleError();
    virtual int HandleWrite();
    virtual int HandleTimeout();
    const ARP::arp_pair GetPair() const{return pair;}
    bool Find()const{return find;}
};

ARPHandler::ARPHandler(sockaddr_in a_ip, ether_addr a_mac, const std::string& dest, 
    const std::string& a_interface)
    : FdHandler(-1, true), ip(a_ip), mac(a_mac), interface(a_interface), 
    dest_ip(dest), find(false)
{
    ARP::set_bpf_arp(fd, buffer_length, interface.c_str());
    pair.ip.sin_family = AF_INET;
    bpf_buffer = new char[buffer_length];
}

ARPHandler::~ARPHandler(){delete[] bpf_buffer;}

int ARPHandler::HandleRead()
{
    /*
    if(curr_pair < pairs_size){
        if(DARP::collectresponse(fd, pairs[curr_pair], bpf_buffer, buffer_length)){
            curr_pair++;
            timeout_counter = 0;
        }
    } else {
        sel.EndRun();
    }
    */
    return 0;
}

int ARPHandler::HandleWrite()
{
//    DARP::writequeries(fd, &mac, &ip, dest_ips, pairs_size);
    //this->SetEvents(this->IETEvent);
    //sel.UpdateEvents(this);
    return 0;
}

int ARPHandler::HandleError()
{
    errors::SysRet("%s", __LINE__);
    return 0;
}

int ARPHandler::HandleTimeout()
{
    // this code should be rewrited to HandleRead/Write
    int counter = 0;
    do{
        ARP::writequery(fd, &mac, &ip, dest_ip.c_str());
        find = ARP::collectresponse(fd, pair, bpf_buffer, buffer_length) 
            && dest_ip == inet_ntoa(pair.ip.sin_addr);
        counter++;
    }while(!find && counter < 5);
    return 0;
}

Arper::Arper(Scheduler& m, const std::string& a_in) : ScheduledEvent(m), 
    interface(a_in), init(false)
{
    std::vector<NetDevice> temp = m.GetDevices(); 
    for(std::vector<NetDevice>::iterator it = temp.begin(); it != temp.end();
        it++){
        if(it->GetType() == OwnHost){
           own_dev = *it; 
        }
    }
}

void Arper::Init()
{
    std::vector<NetDevice> temp = master.GetDevices(); 
    for(std::vector<NetDevice>::iterator it = temp.begin(); it != temp.end();
            it++){
        if(!it->HasMac()){
            updated_dev.push_back(*it);
        }
    }
    init = true;
}
void Arper::Act()
{
    if(!init)
        Init();
    if(!updated_dev.empty() && current_index != updated_dev.size()){
        sockaddr_in own_ip;
        ether_addr own_mac;
        own_dev.GetIp(own_ip);    
        own_dev.GetMac(own_mac);
        ARPHandler arp(own_ip, own_mac, updated_dev[current_index].GetIp(),
            interface);
        arp.HandleTimeout();
        if(arp.Find()){
            updated_dev[current_index].SetMac(arp.GetPair().mac);
        }
        current_index++;
    }else{
        master.EndNormalScheduledEvent();
    }
    
}
