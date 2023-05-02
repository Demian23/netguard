#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/ip.h"

namespace IP{

static int itoa(int n, char s[]);
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

std::set<std::string> all_net_ipv4(const std::string& net, uint32_t start,
    uint32_t interval_size)
{
    uint32_t net_number = ipv4_to_number(net);
    net_number += start;
    std::set<std::string> res;
    for(int i = 0; i < interval_size; i++){
        net_number++;
        res.insert(number_to_ipv4(net_number));
    }
    return res; 
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

DevType devtype_from_vendor(const char *vendor)
{
    const char *cp_vendors[] = {"TP-LINK TECHNOLOGIES CO.,LTD.", 
        "Cisco Systems, Inc", "D-Link Corporation"};
    for(int i = 0; i < 1; i++){
        if(strlen(cp_vendors[i]) == strlen(vendor))
            if(strcmp(cp_vendors[i], vendor) == 0)
                return customer_premise;
    }
    return endpoint;
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

static int itoa(int n, char s[])
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
