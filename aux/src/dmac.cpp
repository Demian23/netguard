#include "dmac.hpp"
#include "dneterr.hpp"
#include <cstdio>
#include <cstring>
namespace DMAC{
    enum{first_part_size = 8};    
    const char *vendors_file = "./net_guard/aux_files/mac-vendors-export.csv";
    mac_vendor *get_3octet(const ether_addr *ether_arr, int len);  
    int check_record(mac_vendor *arr, char *buff, int len);
    char **get_tokens(const char *src);
};

DMAC::mac_vendor* DMAC::get_3octet(const ether_addr *ether_arr, int len)
{
    mac_vendor *res = new mac_vendor[len];
    for(int i = 0; i < len; i++){
        res[i].eth = ether_arr[i];
        res[i].octets = new char[first_part_size];
        res[i].find = false;
        res[i].info = 0;
        sprintf(res[i].octets, "%02X:%02X:%02X", ether_arr[i].octet[0], 
            ether_arr[i].octet[1], ether_arr[i].octet[2]); 
    }
    return res;
}


DMAC::mac_vendor* DMAC::vendors_arr(const ether_addr *arr, int len)
{
    mac_vendor *res = get_3octet(arr, len);
    FILE *vf = fopen(vendors_file, "r");
    if(vf == NULL)
        DERR::Sys("can't open file");
    int counter = 0; 
    bool endflag = false;
    while(!endflag && (counter < len)){
        char buff[100] = {};
        endflag = fgets(buff, 100, vf) == NULL;
        if(!endflag)
            counter += check_record(res, buff, len) ; 
    }
    fclose(vf);
    return res;
}

int DMAC::check_record(mac_vendor *arr, char *buff, int len)
{
    int coutner = 0;
    for(int i = 0; i < len; i++){
        if(!arr[i].find && (strncmp(arr[i].octets, buff, first_part_size) == 0)){
            arr[i].find = true; 
            arr[i].info = get_tokens(buff + first_part_size + 1);
            coutner++; 
        }
    } 
    return coutner;
}


char** DMAC::get_tokens(const char *src)
{
    char **res = new char*[mac_vendor::info_size];
    int offset = 0;
    int len;
    char buff[256] = {};
    for(int i = 0; i < mac_vendor::info_size; i++){
        if(i == 0 && src[0] == '"'){
            sscanf(src + offset + 1, "%[^\"]%*c", buff);
            offset += 2;
        } else {
            sscanf(src + offset, "%[^,\n]%*c", buff);
        }
        len = strlen(buff);
        res[i] = new char[len + 1];
        strcpy(res[i], buff);
        offset += len + 1;
        memset(buff, 0, len);
    }
    return res;
}
