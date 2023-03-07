#include "dmac.hpp"
#include "dneterr.hpp"
#include <cstdio>
#include <cstring>
namespace DMAC{
    enum{lines_amount = 48169, first_part_size = 8};    
    const char *vendors_file = "mac-vendors-export.csv";
    mac_vendor *get_3octet(const ether_addr *ether_arr, int len);  
    int check_record(mac_vendor *arr, char *buff, int len);
};

DMAC::mac_vendor* DMAC::get_3octet(const ether_addr *ether_arr, int len)
{
    mac_vendor *res = new mac_vendor[len];
    for(int i = 0; i < len; i++){
        res[i].eth = ether_arr[i];
        res[i].vendor = new char[first_part_size];
        res[i].find = false;
        sprintf(res[i].vendor, "%02X:%02X:%02X", ether_arr[i].octet[0], 
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
    int size = strlen(buff) - first_part_size - 1 - 1;
    for(int i = 0; i < len; i++){
        if(!arr[i].find && (strncmp(arr[i].vendor, buff, first_part_size) == 0)){
            arr[i].find = true; 
            delete[] arr[i].vendor;
            arr[i].vendor = new char[size];
            memcpy(arr[i].vendor, buff + first_part_size + 1, size);
            coutner++; 
        }
    } 
    return coutner;
}
