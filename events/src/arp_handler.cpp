#include "arp_handler.hpp"

ARPHandler::ARPHandler(sockaddr_in a_ip, ether_addr a_mac, char **ips, 
    const int len, const char *a_interface, EventSelector &a_sel)
    : FdHandler(-1, true), sel(a_sel), ip(a_ip), mac(a_mac), interface(a_interface), 
    dest_ips(ips), pairs_size(len), timeout_counter(0)
{
    DARP::set_bpf_arp(fd, buffer_length, interface);
    DARP::set_timeout(fd);
    pairs = new DARP::arp_pair[pairs_size];
    bpf_buffer = new char[buffer_length];
}

ARPHandler::~ARPHandler()
{
    delete[] bpf_buffer;
    delete[] pairs;
}

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
    DERR::Sys("Some error on ARPHandler");
    return 0;
}

int ARPHandler::HandleTimeout()
{
    for(int i = 0; i < pairs_size; i++){
        timeout_counter = 0;
        while(timeout_counter < 5){
            DARP::writequery(fd, &mac, &ip, dest_ips[i]);
            bool endflag= DARP::collectresponse(fd, pairs[i], bpf_buffer, buffer_length);
            char *ip_str = inet_ntoa(pairs[0].ip.sin_addr);
            endflag &= strcmp(dest_ips[i], ip_str) == 0;
            if(endflag)
                break;
            else
                timeout_counter++;
        }
    }
    sel.EndRun();
    return 0;
}
