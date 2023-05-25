#include <netdb.h>
#include <sstream>

#include "../include/errors.h"
#include "../include/ip.h"

namespace IP{

bool is_ip_in_net(const std::string &net, const std::string &mask, const std::string &ip)
{
   uint32_t first_ip_n = ipv4_to_number(first_ip(net));
   uint32_t last_ip_n = ipv4_to_number(last_ip(net, mask));
   uint32_t ip_n = ipv4_to_number(ip);
   return ip_n >= first_ip_n && ip_n <= last_ip_n;
}

bool is_valid_ip_string(const std::string& ip_string)
{
    std::stringstream stream(ip_string);
    std::string temp;
    bool res = true;
    for(int i = 0; res && i < 3; i++){
        res = false;
        if(stream.eof() || stream.bad())
            break;
        std::getline(stream, temp, '.');
        if(!stream.bad()){
            int n = atoi(temp.c_str());
            if(n < 256 && n >  0){
                res = true;
            }
        }
    }
    res = false;
    temp.clear();
    std::getline(stream, temp);
    if(!temp.empty()){
        int n = atoi(temp.c_str());
        if(n < 256 && n >  0){
            res = true;
        }
    } 
    return res;
}

bool check_ip_range(const std::string& net, const std::string& mask, const std::string& first,
    const std::string& last)
{
   uint32_t first_ip_n = ipv4_to_number(first_ip(net));
   uint32_t last_ip_n = ipv4_to_number(last_ip(net, mask));
   uint32_t first_n = ipv4_to_number(first);
   uint32_t last_n = ipv4_to_number(last);
   if(first_n >= first_ip_n && first_n <= last_ip_n && last_n >= first_ip_n 
       && last_n <= last_ip_n && first_n <= last_n)
       return true;
   else return false;
}

std::vector<std::string> all_ipv4_from_range(const std::string &first, const std::string &last)
{
    uint32_t first_ip = ipv4_to_number(first);
    uint32_t last_ip = ipv4_to_number(last);
    std::vector<std::string> res;
    for(uint32_t iter = first_ip; iter <= last_ip; iter++){
        res.push_back(number_to_ipv4(iter));
    }
    return res;
}

std::string get_name(sockaddr_in* addr)
{
    char buff[NI_MAXHOST];
    getnameinfo(reinterpret_cast<const sockaddr*>(addr), sizeof(sockaddr_in), 
        buff, NI_MAXHOST, 0, 0, 0);
    return buff;
}

in_addr str_to_ip(const char *ip_str)
{
    in_addr res;
    if(!inet_aton(ip_str, &res))
        errors::SysRet("");
    return res;
}

short mask_prefix(const std::string& mask)
{
    short mask_len = 0;
    uint32_t mask_n = ipv4_to_number(mask);
    for(int j = 0; j < 32; j++)
        mask_len += (mask_n >> j) & 0x01;
    return mask_len;
}

uint32_t mask_number(const short mask_prefix)
{
    uint32_t m = 1;
    for(int i = 0; i < mask_prefix - 1; i++){
       m <<= 1; 
       m += 1;
    }
    m <<= 32 - mask_prefix;
    return m;
}

uint32_t ip_amount(const short mask_prefix)
{
    uint32_t res = (1 << (32 - mask_prefix)) - 2;
    return res; 
}

//mask in prefix format as 192.168.1.1/24, where 24 -> mask
std::string ipv4_net(const std::string& some_ip, const short mask_prefix)
{
    uint32_t ip_n = ipv4_to_number(some_ip);
    uint32_t m = mask_number(mask_prefix);
    return number_to_ipv4(ip_n & m);
}

std::string ipv4_net(const std::string& some_ip, const std::string& mask)
{
    uint32_t ip_n = ipv4_to_number(some_ip);
    uint32_t mask_n = ipv4_to_number(mask);
    return number_to_ipv4(ip_n & mask_n);
}

std::string first_ip(const std::string& net)
{
    return number_to_ipv4(ipv4_to_number(net)+1);
}

std::string last_ip(const std::string& net, const std::string& mask)
{
    int interval_size = ip_amount(mask_prefix(mask));    
    return number_to_ipv4(ipv4_to_number(net) + interval_size);
}

uint32_t ipv4_to_number(const std::string& ip)
{
    uint32_t res = 0; int start = 0, end = 0;
    for(int i = 0; i < 3; i++){
        end = ip.find('.', start);
        res |= std::stoi(ip.substr(start, end));
        res <<= 8;
        start = end + 1;
    }
    res |= std::stoi(ip.substr(start, ip.size()));
    return res;
}

std::string number_to_ipv4(const uint32_t number)
{
    char buff[4][4] = {};    
    uint32_t temp = number;
    for(int i = 3; i >= 0; i--, temp >>= 8){
        itoa(temp & 0xFF, buff[i]);
    }
    return std::string(buff[0]) + "." + buff[1] + "." + buff[2] + "." + buff[3];
}

static void reverse(char s[])
{
	int i, j;
	char c;
	
	for(i = 0, j = strlen(s)-1; i<j; i++, j--){
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

int itoa(int n, char s[])
{
	int i, sign;

	if((sign = n) < 0)
		n = -n;
	i = 0;
	do{
		s[i++] = n % 10 + '0';
	}while((n /= 10) > 0);
	if(sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
	return i;
}
}
