#include "pinger.h"
#include "../../net/src/errors.h"
#include "../../net/src/host_addr.h"
#include "../../net/src/icmp.h"
#include <fcntl.h>

class SendEcho : public FdHandler{
private:
    Pinger& father;
    const std::string& dest_ip;
    int id;
public:
    SendEcho(int a_fd, bool own, const std::string& ip, int a_id, Pinger& f);
    const std::string& GetDestIp() const{return dest_ip;}
    virtual int HandleTimeout();
    virtual int HandleError();
    virtual int HandleRead();
    virtual int HandleWrite();
    virtual ~SendEcho(){}
};

class RecvEcho: public FdHandler{
private:
    std::set<std::string> ips;
    msghdr msg;
public:
    RecvEcho(int a_fd, bool own);
    virtual int HandleError();
    virtual int HandleRead();
    virtual int HandleWrite();
    virtual int HandleTimeout();
    const std::set<std::string>& GetIps() const{return ips;}
    virtual ~RecvEcho(){}
};


SendEcho::SendEcho(int a_fd, bool own, const std::string& ip, int a_id, Pinger& f)
    : FdHandler(a_fd, own), father(f), dest_ip(ip), id(a_id)
{}

int SendEcho::HandleRead()
{
    errors::Msg("Try to read from SendEcho");
    return -1;
}

int SendEcho::HandleError()
{
    errors::SysRet("Some error on SendEcho");
    return 0;
}

int SendEcho::HandleWrite()
{
    sockaddr_in dest_addr = host_addr::set_addr(dest_ip.c_str(), AF_INET);
    ICMP::send_echo(fd, id, 0, &dest_addr, sizeof(dest_addr));
    SetEvents(None);
    father.UpdateHandlerEvents(this);
    ExplicitlyEnd();
    return 0;
}

int SendEcho::HandleTimeout()
{
    return -1;
}

RecvEcho::RecvEcho(int a_fd, bool own)
    : FdHandler(a_fd, own)
{
    memset(&msg, 0, sizeof(msg)); 
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int RecvEcho::HandleError()
{
    errors::SysRet("Some errror on RecvEcho");  
    return 0;
}

int RecvEcho::HandleRead()
{
    bool end_read;
    do{
        ssize_t len = ICMP::recv_reply(fd , &msg);
        end_read = len == -1;
        if(!end_read){
            in_addr ip;
            ICMP::Errors result = ICMP::get_echo(
                    reinterpret_cast<char *>(msg.msg_iov->iov_base),len, ip);
            if(result == ICMP::Allright)
                ips.insert(inet_ntoa(ip));
        }
        delete[] msg.msg_iov; delete[] (char*) msg.msg_control;
    }while(!end_read);
    return 0;
}

int RecvEcho::HandleWrite()
{
    errors::Msg("Try to write form RecvEcho");
    return -1;
}

int RecvEcho::HandleTimeout()
{
    return -1;
}

Pinger::Pinger(Scheduler& m, const std::set<std::string>& ip_set)
    : ScheduledEvent(m), ping_ips(ip_set), id(ICMP::get_id())
{
    poll_size = 8; // TODO:make it adaptive to size of network
    it = ping_ips.begin(); 
    send_icmp_sd= new int[poll_size];
    send_handlers = new SendEcho*[poll_size]; 
}

Pinger::~Pinger()
{
    delete[] send_handlers;
    delete[] send_icmp_sd;
}

void Pinger::Act()
{
    //first check if we can create new send/echo 
    //no, than skeep
    if(send_handlers[0] != 0){
        if(!CreateHandlers()){
            UpdateDevices();            
            master.EndNormalScheduledEvent();
        }
    } else {
        CreateHandlers();
        int reciver_fd;
        if(ICMP::make_icmp_socket(reciver_fd))
            reciver = new RecvEcho(reciver_fd, true);
        else 
            errors::Sys("Pinger::Act");
        reciver->SetEvents(FdHandler::InEvent + FdHandler::ErrEvent);
        master.AddHandler(reciver);
    }
}

bool Pinger::CreateHandlers()
{
    bool res = false;
    if(it != ping_ips.end()){
        for(int i = 0; it != ping_ips.end() && i < poll_size; it++, i++){
            if(!ICMP::make_icmp_socket(send_icmp_sd[i]))
                errors::Sys("ICMP socket not created.");
            send_handlers[i] = new SendEcho(send_icmp_sd[i], true, 
                *it, id, *this);
            send_handlers[i]->SetEvents(FdHandler::OutEvent + FdHandler::ErrEvent);
            master.AddHandler(send_handlers[i]);
            id = ICMP::get_id();
        }
        res = true;
    }
    return res;
}

void Pinger::UpdateHandlerEvents(FdHandler *h)
{
    master.UpdateHandlerEvents(h);
}

void Pinger::UpdateDevices()
{
    std::vector<NetDevice>& temp = master.GetDevices();
    const std::set<std::string>& ips_set = reciver->GetIps();
    std::set<std::string>::const_iterator ips_it = ips_set.begin();
    for(;ips_it != ips_set.end(); ips_it++){
        NetDevice new_dev; 
        new_dev.SetIpv4(*ips_it);
        temp.push_back(new_dev);
    }
    reciver->SetEvents(FdHandler::None);
    UpdateHandlerEvents(reciver);
    reciver->ExplicitlyEnd();
}

