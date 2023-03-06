#include <cstdio>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "../aux/src/dneterr.hpp"
#include "../events/src/socket_handlers.hpp"
#include "../events/src/arp_handler.hpp"
#include "../aux/src/dicmp.hpp"
#include "../aux/src/dip.hpp"
#include "../aux/src/darp.hpp"

enum{default_timeout = 200, too_much = 30};

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

char **get_res(char ***arp_arr, thread_data **ret, int threads_amount, int &len)
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

void *find_ip(void *data)
{
    EventSelector ev;
    thread_data *d = reinterpret_cast<thread_data *>(data);
    int sget = 0, ssend = 0;
    char **ip_array = DIP::all_net_ipv4(d->net, d->interval_start, 
        d->interval_size);
    for(int i = 0; ip_array[i]; i++){
        if(!DICMP::make_icmp_socket(ssend) || !DICMP::make_icmp_socket(sget))
            DERR::Quit("Socket creation mistake");
        SendICMPHandler *send = new SendICMPHandler(ssend, true, ip_array[i], ev, d->id);
        send->SetEvents(FdHandler::Timeout);
        RecvICMPHandler *get = new RecvICMPHandler(sget, true, d->id);
        get->SetEvents(FdHandler::IEEvent);

        ev.Add(send);
        ev.Add(get);
        ev.Run(d->icmp_timeout);
        if(get->Exists()){
            (d->arp_ip_len)++;
        } else {
            delete[] ip_array[i];
            ip_array[i] = 0;
        }
        send->ExplicitlyEnd();
        get->ExplicitlyEnd();
        (d->id)++;
    }
    char **arp_ip = new char*[d->arp_ip_len];
    int j = 0;
    for(int i = 0; i < d->interval_size; i++){
        if(ip_array[i]){
            arp_ip[j] = ip_array[i];
            j++;
        }
    } 
    delete[] ip_array; 
    pthread_exit(reinterpret_cast<void *>(arp_ip));
}

void print_info(sockaddr_in ip, ether_addr mac, const char *name)
{
    printf("%s(%s) is at %02x:%02x:%02x:%02x:%02x:%02x\n",
            inet_ntoa(ip.sin_addr), name,
            mac.octet[0], mac.octet[1], mac.octet[2], 
            mac.octet[3], mac.octet[4], mac.octet[5]);
}

int main(int argc, char **argv)
{
    short mask_prefix = 0;
    char *net = 0;
    struct ether_addr ownmac;
    struct sockaddr_in ip, mask;
    char ip_s[32];
/*
    if(argc == 3){
        mask_prefix = atoi(argv[2]); 
        net = DIP::ipv4_net(argv[1], mask_prefix);
    }else{ 
        if(argc == 2){
            mask_prefix = atoi(argv[1]);
            char *ip = DNET::get_local_ip();
            net = DIP::ipv4_net(ip, mask_prefix);
        }else
*/
        if(argc == 2){
            if(DNET::findownaddr(argv[1], &ownmac, &ip, &mask)){
                char *temp = 0;
                temp = inet_ntoa(ip.sin_addr);
                memcpy(ip_s, temp, strlen(temp));
                temp = inet_ntoa(mask.sin_addr);
                mask_prefix = DIP::mask_prefix(temp); 
                net = DIP::ipv4_net(ip_s, mask_prefix); 
            } else {
                DERR::Quit("Can't find interface");
            }
        }else{
           DERR::Quit("usage: %s <interface>", argv[0]); 
        }
    if(mask_prefix == 0)
        DERR::Quit("Wrong input, mask must be in prefix form");
    if(net == 0)
        DERR::Quit("Wrong ip address");


    int id = getpid() & 0xFFFF;

    uint32_t interval_size = DIP::ip_amount(mask_prefix);
    int threads_amount = 1;
    if(interval_size > too_much){ 
        interval_size >>= 2;
        threads_amount = 4;
    }

    thread_data **threads_return = new thread_data*[threads_amount];
    pthread_t *threads_arr = new pthread_t[threads_amount];
    int addition = 0;
    int timeout = default_timeout;

    for(int i = 0; i < threads_amount; i++){
        id += addition;
        threads_return[i] = new thread_data(net, id, addition, interval_size, timeout); 
        addition += interval_size;
        pthread_create(&threads_arr[i], NULL, find_ip, 
            reinterpret_cast<void *>(threads_return[i]));
    }

    char ***arp_ip_arr = new char**[threads_amount];
    for(int i = 0; i < threads_amount; i++){
        void *res = 0;
        pthread_join(threads_arr[i], &res);
        arp_ip_arr[i] = reinterpret_cast<char **>(res);
    }
    setuid(getuid());
    delete[] net;
    delete[] threads_arr;
    int len = 0;
    char **arp_ips = get_res(arp_ip_arr, threads_return, threads_amount, len);
    printf("Own address:\n");
    struct addrinfo *info, hints;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = AF_INET;
    if(!getaddrinfo(ip_s, NULL, &hints, &info)){
        print_info(ip, ownmac, info->ai_canonname);
        freeaddrinfo(info);
    } else {
        print_info(ip, ownmac, "no name");
    }
    putchar('\n');
    EventSelector ev;
    for(int i = 0; i < len; i++){
        if(strcmp(arp_ips[i], ip_s) == 0)
            continue;
        ARPHandler *arp_h = new ARPHandler(ip, ownmac, arp_ips + i, 
            1, argv[1], ev);
        arp_h->SetEvents(FdHandler::Timeout + FdHandler::ErrEvent);
        ev.Add(arp_h);
        ev.Run(1);
        const DARP::arp_pair *p = arp_h->GetPairs();
        if(!getaddrinfo(ip_s, NULL, &hints, &info)){
            print_info(p->ip, p->mac, info->ai_canonname);
            freeaddrinfo(info);
        } else {
            print_info(p->ip, p->mac, "no name");
        }
        arp_h->ExplicitlyEnd();
    }
    return 0;
}
