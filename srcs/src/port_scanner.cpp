#include "../include/errors.h"
#include "../include/ip.h"
#include "../include/port_scanner.h"


class TcpHandshakeAnalizer : public IEvent{
    ports_storage& ports;
    in_addr from;
    bpf_hdr* bpf_buffer;
    int bpf_buffer_size;
    int bpf_fd;
   // int manual_sd;
    bool end;
public:
    TcpHandshakeAnalizer(in_addr a_from, ports_storage& a_ports)
        :  ports(a_ports), from(a_from),  bpf_fd(0), end(false) {}
    bool Init(const char* net_interface);
    void ProcessBpfPacket(bpf_hdr* packet);
    void SetEnd(){end = true;}
    bool End()const override{return end;}
    void OnError() override{errors::SysRet("ERROR: on analyzing tcp packet"); end = true;}
    short ListeningEvents()const override{return Any;} //Error;}
    int GetDescriptor() const override{return bpf_fd;}
    void OnAnyEvent() override;
    virtual ~TcpHandshakeAnalizer(){close(bpf_fd); delete[] bpf_buffer;}
};

static int find_bpf_device()
{
    int res;
    char dev[12] = {};
    for(int i = 0; i < 255; i++){
        snprintf(dev, 12, "/dev/bpf%d", i);
        res = open(dev, O_RDWR); 
        if(res > 0)
            break;
    }
    return res;
}

bool TcpHandshakeAnalizer::Init(const char* net_interface)
{
    bool res = false;
   //if(raw_packets::make_manual_socket(manual_sd, IPPROTO_TCP)){
    bpf_fd = find_bpf_device();
    if(bpf_fd != -1){
struct bpf_insn insns[] = {
    // Load word at octet 12
    BPF_STMT(BPF_LD | BPF_H | BPF_ABS, 12),
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, ETHERTYPE_IP, 0, 3),

    BPF_STMT(BPF_LD | BPF_B | BPF_ABS, 23),
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, IPPROTO_TCP, 0, 1),
    // Valid TCP reply received, return message
    BPF_STMT(BPF_RET | BPF_K, sizeof(tcphdr) + sizeof(ip) + sizeof(ether_header)),
    // Return nothing
    BPF_STMT(BPF_RET | BPF_K, 0),
};
struct bpf_program filter = {
    sizeof insns / sizeof(insns[0]),
    insns
};
        struct ifreq ir;
        bpf_buffer_size = 1;
        strncpy(ir.ifr_name, net_interface, IFNAMSIZ);
        res = ioctl(bpf_fd, BIOCSETIF, &ir) != -1 && 
        ioctl(bpf_fd, BIOCIMMEDIATE, &bpf_buffer_size) != -1 &&
        ioctl(bpf_fd, BIOCGBLEN, &bpf_buffer_size) != -1;
        timeval timeout = {.tv_sec = 0, .tv_usec = 1000};
        ioctl(bpf_fd, BIOCSRTIMEOUT, &timeout);
        res = ioctl(bpf_fd, BIOCSETF, &filter) != -1;
    }
   bpf_buffer = (bpf_hdr*)new char[bpf_buffer_size];
   return res;
}

void TcpHandshakeAnalizer::ProcessBpfPacket(bpf_hdr *packet)
{
    ip* ip_header = (ip*)((char*) packet + packet->bh_hdrlen + 14);
    if(ip_header->ip_src.s_addr == from.s_addr){
        tcphdr* tcp_header = (tcphdr*)((char*) packet + packet->bh_hdrlen + 34);
        uint16_t port = ntohs(tcp_header->th_sport);    
        ports_storage::iterator it = ports.find(port);
        if(it != ports.end()){
            if(tcp_header->th_flags & TH_ACK && tcp_header->th_flags & TH_SYN){
                it->second = Open;
                /*
                 * as I understand, os by itself sending rst
                sockaddr_in src; src.sin_family = AF_INET; 
                src.sin_addr = ip_header->ip_dst; src.sin_port = tcp_header->th_dport;
                sockaddr_in dest; dest.sin_family = AF_INET;
                dest.sin_addr = ip_header->ip_src; dest.sin_port = tcp_header->th_sport;
                char rst_buffer[1024] = {};
                raw_packets::send_tcp_flag(manual_sd, &src, 
                    &dest, TH_RST, rst_buffer);
                */
            }
            if(tcp_header->th_flags & TH_RST){
                it->second = Closed;
            }
        }
    }
}

void TcpHandshakeAnalizer::OnAnyEvent()
{
    if(!end){
        int read_bytes;
        bpf_hdr* bpf_packet;
        memset(bpf_buffer, 0, bpf_buffer_size);
        if((read_bytes = read(bpf_fd, bpf_buffer, bpf_buffer_size)) > 0){
            char* ptr = reinterpret_cast<char*>(bpf_buffer);
            while(ptr < reinterpret_cast<char*>(bpf_buffer) + read_bytes){
                bpf_packet = reinterpret_cast<bpf_hdr*>(ptr);
                ProcessBpfPacket(bpf_packet);
                ptr += BPF_WORDALIGN(bpf_packet->bh_hdrlen + bpf_packet->bh_caplen);
            }
        }
    }
}

PortScanner::PortScanner(Scheduler& a_master, const ports_storage& a_ports, 
    const std::string& dest_ip_str, Statistic* stat)
    : master(a_master), ports(a_ports), statistic(stat), wait_counter(0), scanners_size(0)
{
    inet_aton(dest_ip_str.c_str(), &dest.sin_addr);
    dest.sin_family = AF_INET;
    ports_it = ports.begin();
    const NetNode& temp = master.manager.GetOwnNode();
    inet_aton(temp.ipv4_address.c_str(), &src.sin_addr); 
    src.sin_port = htons(49000); src.sin_family = AF_INET;
    if(ports.size() > 1024)
        scanners_size = (ports.size() / 1024) + 1;
    else scanners_size = 5;
}

bool PortScanner::CheckPreviousPorts()
{
    bool result = true;
    if(!previous_ports.empty()){
        for(std::vector<uint16_t>::const_iterator it = previous_ports.begin();        
            it != previous_ports.end();){
            if(ports[*it] == Unset){
                if(++wait_counter >= 5)
                    ports[*it] = Filtered;
                else
                    result = false;
                it++;
            } else {
                it = previous_ports.erase(it);
            }
        }
    }
    return result; 
}

bool PortScanner::UrgentExecute()
{
    bool result = false;    
    if(ports_it == ports.begin()){
        if(!raw_packets::make_manual_socket(manual_sd, IPPROTO_TCP))
            return true;
        for(int i = 0; i < scanners_size / 8 + 1; i++){
            TcpHandshakeAnalizer* reciver = new TcpHandshakeAnalizer(dest.sin_addr, ports);
            if(reciver->Init(master.manager.GetInterface().c_str())){
                master.AddToSelector(reciver);
                recivers.push_back(reciver);
            }
            else delete reciver;
        }
    }
    if(master.manager.IsPortScanStop()){
        ports_it = ports.end();
        master.manager.StoppedPortScan();
    }
    if(CheckPreviousPorts()){
        char packet_buffer[1024] = {};
        wait_counter = 0;
        if(ports_it != ports.end()){
            for(int i = 0; ports_it != ports.end() && i < scanners_size; ports_it++, i++){
                dest.sin_port = htons(ports_it->first);
                raw_packets::send_tcp_flag(manual_sd, &src, &dest, TH_SYN, packet_buffer);
                previous_ports.push_back(ports_it->first);
            }
            src.sin_port++;
            if(statistic)
                statistic->RecordStatistic(this);
        } else {
            std::for_each(recivers.begin(), recivers.end(), 
                [](TcpHandshakeAnalizer* r){r->SetEnd();});
            master.manager.AddPorts(inet_ntoa(dest.sin_addr), ports);
            close(manual_sd);
            if(statistic)delete statistic;
            result = true;
        }
    }
    return result;
}

int PortScanner::GetCurrentCount()const
{
    return std::distance<ports_storage::const_iterator>(ports.begin(), ports_it);
}

std::string PortScanner::GetAim() const
{
    return inet_ntoa(dest.sin_addr);
}
