#include "arp_handler.h"
#include "../../aux/src/errors.h"
#include <arpa/inet.h>

ARPHandler::ARPHandler(sockaddr_in a_ip, ether_addr a_mac, const std::string& dest, 
    const std::string& a_interface, EventSelector &a_sel)
    : FdHandler(-1, true), sel(a_sel), ip(a_ip), mac(a_mac), interface(a_interface), 
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
    sel.EndRun();
    return 0;
}
