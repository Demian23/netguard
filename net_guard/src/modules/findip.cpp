#include "findip.hpp"

#include "../../../aux/src/dneterr.hpp"
#include "../../../events/src/socket_handlers.hpp"
#include "../../../aux/src/dip.hpp"

static int get_poll_am(const int interval_size)
{
    int ip_am= 1;
    if(interval_size > 0x0F)
        ip_am <<= 1;
    if(interval_size > 0x80)
        ip_am <<= 1;
    if(interval_size > 0xFF)
        ip_am <<= 1;
    return ip_am << 1;
}

static char **get_res_arr(const int res_len, const int src_size, char **src)
{
    char **res = new char*[res_len];
    int j = 0;
    for(int i = 0; i < src_size; i++){
        if(src[i]){
            res[j] = src[i];
            j++;
        }
    } 
    delete[] src;
    return res;
}

char **get_real_ip(const char *net, const short mask_prefix, int &res_len)
{
    enum{def_timeout = 200};
    int id = getpid() & 0xFFFF;
    int arp_ip_len = 0;
    int interval_size = DIP::ip_amount(mask_prefix);
    char **ip_array = DIP::all_net_ipv4(net, 0, 
            interval_size);
    const int poll_am = get_poll_am(interval_size);
    EventSelector ev;
    int *sd_arr = new int[poll_am];
    FdHandler **handlers_arr = new FdHandler*[poll_am];

    for(int j = 0; j < interval_size; j += poll_am >> 1){
        for(int i = 0; i < poll_am && j + (i >> 1) < interval_size; i += 2){
            if(!DICMP::make_icmp_socket(sd_arr[i]) || 
                !DICMP::make_icmp_socket(sd_arr[i+1]))
                DERR::Quit("Socket creation mistake");
            
            handlers_arr[i] = new SendICMPHandler(sd_arr[i], true, 
                ip_array[j+(i >> 1)], ev, id);
            handlers_arr[i]->SetEvents(FdHandler::Timeout + FdHandler::ErrEvent);

            handlers_arr[i+1] = new RecvICMPHandler(sd_arr[i+1], true, id);
            handlers_arr[i+1]->SetEvents(FdHandler::InEvent + FdHandler::ErrEvent);
            
            ev.Add(handlers_arr[i]);
            ev.Add(handlers_arr[i+1]);
            id++;
        }
        ev.Run(def_timeout);
        for(int i = 0; i < poll_am && j + (i >> 1) < interval_size; i += 2){
            if(static_cast<RecvICMPHandler *>(handlers_arr[i+1])->Exists()){
                (arp_ip_len)++;
            } else {
                delete[] ip_array[j+(i >> 1)];
                ip_array[j+(i >> 1)] = 0;
            }
            handlers_arr[i]->ExplicitlyEnd();
            handlers_arr[i+1]->ExplicitlyEnd();
        }
    }

    setuid(getuid());
    res_len = arp_ip_len;
    char **arp_ip = get_res_arr(res_len, interval_size, ip_array);
    delete[] handlers_arr;
    delete[] sd_arr;
    return arp_ip;
}
