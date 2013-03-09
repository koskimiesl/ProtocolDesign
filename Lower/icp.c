#include"icp.h"

int whichEndian(){
    unsigned short num = 0x00FF;
    char * ptr = (char *)&num;
    if(*ptr == 0x00)
        return 0;   /* Big Endian */
    else
        return 1;   /* Little Endian */
}

void to_binary(char buffer[],struct icp_struct * icp){

    char * ptr;
    buffer[0] = (icp->version << 6)|(icp->startbit << 5)|(icp->endbit << 4)|(icp->ackbit << 3)|(icp->cackbit << 2);
    buffer[1] = 0;
    ptr = (char *)&icp->size;
    buffer[2 + whichEndian()] = *ptr;
    buffer[3 - whichEndian()] = *(ptr+1);
    ptr = (char *)&icp->seq;
    buffer[4 + whichEndian()] = *ptr;
    buffer[5 - whichEndian()] = *(ptr+1);
    ptr = (char *)&icp->ack;
    buffer[6 + whichEndian()] = *ptr;
    buffer[7 - whichEndian()] = *(ptr+1);   
}

void to_struct(char buffer[],struct icp_struct * icp){

    char * ptr;
    icp->version = (buffer[0]&0xC0) >> 6;
    icp->startbit = (buffer[0]&0x20) >> 5;
    icp->endbit = (buffer[0]&0x10) >> 4;
    icp->ackbit = (buffer[0]&0x08) >> 3;
    icp->cackbit = (buffer[0]&0x04) >>2;
    ptr = (char *)&icp->size;
    *(ptr + whichEndian()) = buffer[2];
    *(ptr + 1 - whichEndian()) = buffer[3];
    ptr = (char *)&icp->seq;
    *(ptr + whichEndian()) = buffer[4];
    *(ptr + 1 - whichEndian()) = buffer[5];
    ptr = (char *)&icp->ack;
    *(ptr + whichEndian()) = buffer[6];
    *(ptr + 1 - whichEndian()) = buffer[7];
}
