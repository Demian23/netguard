#include "findip.hpp"

#include <pthread.h>
#include <stdint.h>
#include "../../aux/src/dneterr.hpp"
#include "../../events/src/socket_handlers.hpp"
#include "../../aux/src/dip.hpp"

enum findip_params{default_timeout = 200, too_much = 30, default_threads = 4};
struct thread_data{
    const char *net;
    uint32_t interval_start;
    uint32_t interval_size;
    int id;
    unsigned int arp_ip_len;
    int icmp_timeout;
    thread_data(const char *n, int a_id, uint32_t start, uint32_t size, int t)
        : net(n), interval_start(start), interval_size(size),
        id(a_id), arp_ip_len(0), icmp_timeout(t){}
};

static char **res_in_one_arr(char ***arp_arr, thread_data **ret, int threads_amount, int &len)
{
    len = 0;
    for(int i = 0; i < threads_amount; i++)
        len += ret[i]->arp_ip_len;
    char **res = new char *[len];
    int z = 0;
    for(int i = 0; i < threads_amount; i++){
        for(int j = 0; j < ret[i]->arp_ip_len; j++)
            res[z++] = arp_arr[i][j];
        delete arp_arr[i];
        delete[] ret[i]; 
    }
    delete[] arp_arr;
    delete[] ret;
    return res;
}

static void *thread_proc(void *data)
{
    EventSelector ev;
    thread_data *thr_data = reinterpret_cast<thread_data *>(data);
    int sget = 0, ssend = 0;
    char **ip_array = DIP::all_net_ipv4(thr_data->net, thr_data->interval_start, 
        thr_data->interval_size);
    for(int i = 0; ip_array[i]; i++){
        if(!DICMP::make_icmp_socket(ssend) || !DICMP::make_icmp_socket(sget))
            DERR::Quit("Socket creation mistake");
        SendICMPHandler *send = new SendICMPHandler(ssend, true, ip_array[i], ev, 
            thr_data->id);
        send->SetEvents(FdHandler::Timeout + FdHandler::ErrEvent);
        RecvICMPHandler *get = new RecvICMPHandler(sget, true, thr_data->id);
        get->SetEvents(FdHandler::InEvent + FdHandler::ErrEvent);
        ev.Add(send);
        ev.Add(get);
        ev.Run(thr_data->icmp_timeout);
        if(get->Exists()){
            (thr_data->arp_ip_len)++;
        } else {
            delete[] ip_array[i];
            ip_array[i] = 0;
        }
        send->ExplicitlyEnd();
        get->ExplicitlyEnd();
        (thr_data->id)++;
    }
    char **arp_ip = new char*[thr_data->arp_ip_len];
    int j = 0;
    for(int i = 0; i < thr_data->interval_size; i++){
        if(ip_array[i]){
            arp_ip[j] = ip_array[i];
            j++;
        }
    } 
    delete[] ip_array; 
    pthread_exit(reinterpret_cast<void *>(arp_ip));
}

static int set_threads_amount(const short mask_prefix, uint32_t &interval_size)
{
    interval_size = DIP::ip_amount(mask_prefix);
    int res = 1;
    if(interval_size > too_much){
        interval_size >>= 2;
        res = default_threads;
    }
    return res;
}

char **get_real_ip(const char *net, const short mask_prefix, int &res_len)
{
    uint32_t interval_size;
    int threads_amount = set_threads_amount(mask_prefix, interval_size);
    thread_data **threads_return = new thread_data*[threads_amount];
    pthread_t *threads_arr = new pthread_t[threads_amount];
    int id = getpid() & 0xFFFF;
    int addition = 0;
    int timeout = 200;

    for(int i = 0; i < threads_amount; i++){
        id += addition;
        threads_return[i] = new thread_data(net, id, addition, interval_size, timeout); 
        addition += interval_size;
        pthread_create(&threads_arr[i], NULL, thread_proc,
            reinterpret_cast<void *>(threads_return[i]));
    }

    char ***arp_ip_arr = new char**[threads_amount];
    for(int i = 0; i < threads_amount; i++){
        void *res = 0;
        pthread_join(threads_arr[i], &res);
        arp_ip_arr[i] = reinterpret_cast<char **>(res);
    }
    setuid(getuid());
    char **res = res_in_one_arr(arp_ip_arr, threads_return, threads_amount, 
        res_len);
    delete[] threads_arr;
    delete[] net;
    return res;
}

