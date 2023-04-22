#include "pinger.h"
#include "../../net/src/errors.h"
#include "../../net/src/host_addr.h"
#include "../../net/src/icmp.h"

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
    Pinger& father;
    msghdr msg;
    bool recieved;
    int timeout_counter;
    int id;
public:
    RecvEcho(int a_fd, bool own, int a_id, Pinger& f);
    virtual int HandleError();
    virtual int HandleRead();
    virtual int HandleWrite();
    virtual int HandleTimeout();
    bool Exists() const {return recieved;}
    virtual ~RecvEcho(){} //wrong need to free at least buffers in msg
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
    errors::Msg("Send");
    SetEvents(None);
    father.UpdateHandlerEvents(this);
    return 0;
}

int SendEcho::HandleTimeout()
{
    errors::Msg("Try to timeout from SendEcho");
    return -1;
}


RecvEcho::RecvEcho(int a_fd, bool own, int a_id, Pinger& f)
    : FdHandler(a_fd, own), father(f), recieved(false), id(a_id)
{
   memset(&msg, 0, sizeof(msg)); 
}

int RecvEcho::HandleError()
{
    errors::SysRet("Some errror on RecvEcho");  
    return 0;
}

int RecvEcho::HandleRead()
{
    ssize_t len = ICMP::recv_reply(fd , &msg);
    if(len > 0){
        if(ICMP::process_reply(fd, reinterpret_cast<char *>(msg.msg_iov->iov_base), len, id))
            recieved = true;
        errors::Msg("Read");
        SetEvents(None);
        father.UpdateHandlerEvents(this);
    }
    return 0;
}

int RecvEcho::HandleWrite()
{
    errors::Msg("Try to write form RecvEcho");
    return -1;
}

int RecvEcho::HandleTimeout()
{
    timeout_counter++;
    if(timeout_counter > 3){
        SetEvents(None);
        father.UpdateHandlerEvents(this);
    }
    return 0;
}

Pinger::Pinger(Scheduler& m, const std::set<std::string>& ip_set)
    : ScheduledEvent(m), ping_ips(ip_set), id(ICMP::get_id())
{
    poll_size = 8; // TODO:make it adaptive to size of network
    it = ping_ips.begin(); 
    send_icmp_sd= new int[poll_size];
    recieve_icmp_sd = new int[poll_size];
    send_handlers = new SendEcho*[poll_size]; 
    recieve_hanlders = new RecvEcho*[poll_size];
}

Pinger::~Pinger()
{
    delete[] send_handlers;
    delete[] recieve_hanlders;
    delete[] send_icmp_sd;
    delete[] recieve_icmp_sd;
}

void Pinger::Act()
{
    //first check if we can create new send/echo 
    //no, than skeep
    if(send_handlers[0] != 0){
        if(HandlersDone()){
            GainResults();
            if(!CreateHandlers()){
                // Update devices vector
                UpdateDevices();            
                master.EndNormalScheduledEvent();
            }
        }
    } else {
        CreateHandlers();
    }
}

bool Pinger::HandlersDone() const
{
    bool res = true;
    for(int i = 0; res && i < poll_size; i++){
        res &= send_handlers[i]->GetEvents() == SendEcho::None;
        res &= recieve_hanlders[i]->GetEvents() == RecvEcho::None;
    }
    return res;
}

bool Pinger::CreateHandlers()
{
    bool res = false;
    if(it != ping_ips.end()){
        for(int i = 0; it != ping_ips.end() && i < poll_size; it++, i++){
            if(!ICMP::make_icmp_socket(send_icmp_sd[i])
            || !ICMP::make_icmp_socket(recieve_icmp_sd[i]))
                errors::Sys("ICMP socket not created.");
            send_handlers[i] = new SendEcho(send_icmp_sd[i], true, 
                *it, id, *this);
            recieve_hanlders[i] = new RecvEcho(recieve_icmp_sd[i], true, id, *this);
            send_handlers[i]->SetEvents(FdHandler::OutEvent + FdHandler::ErrEvent);
            recieve_hanlders[i]->SetEvents(FdHandler::Timeout + FdHandler::InEvent + FdHandler::ErrEvent);
            master.AddHandler(send_handlers[i]);
            master.AddHandler(recieve_hanlders[i]);
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

void Pinger::GainResults()
{
    for(int i = 0; i < poll_size; i++){
        if(recieve_hanlders[i]->Exists()){
            real_ip.push_back(send_handlers[i]->GetDestIp());
        }
        recieve_hanlders[i]->ExplicitlyEnd();
        send_handlers[i]->ExplicitlyEnd();
    }
}

void Pinger::UpdateDevices()
{
    std::vector<NetDevice>& temp = master.GetDevices();
    bool exists;
    for(int i = 0; i < real_ip.size(); i++){ 
        exists = false;
        for(int j = 0; j < temp.size(); j++){
            if(real_ip[i] == temp[j].GetIp()){
                exists = true;
                break;
            }
        }
        if(!exists){
            NetDevice new_dev; new_dev.SetIpv4(real_ip[i]);
            temp.push_back(new_dev);
        }
    }
}
