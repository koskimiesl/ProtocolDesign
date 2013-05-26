#include"icp.h"

int whichEndian(){
    unsigned short num = 0x00FF;
    char * ptr = (char *)&num;
    if(*ptr == 0x00)
        return 0;   /* Big Endian */
    else
        return 1;   /* Little Endian */
}

void toBinary(struct ICP * icp){
    unsigned char * ptr;
    icp->buffer[0] = (icp->version << 6)|(icp->startbit << 5)|(icp->endbit << 4)|(icp->ackbit << 3)|(icp->cackbit << 2)|(icp->kalive << 1);
    icp->buffer[1] = 0;
    ptr = (unsigned char *)&icp->size;
    icp->buffer[2 + whichEndian()] = *ptr;
    icp->buffer[3 - whichEndian()] = *(ptr+1);
    ptr = (unsigned char *)&icp->seq;
    icp->buffer[4 + whichEndian()] = *ptr;
    icp->buffer[5 - whichEndian()] = *(ptr+1);
    ptr = (unsigned char *)&icp->ack;
    icp->buffer[6 + whichEndian()] = *ptr;
    icp->buffer[7 - whichEndian()] = *(ptr+1);   
}

void toValues(struct ICP * icp){
    unsigned char * ptr;
    icp->version = (icp->buffer[0]&0xC0) >> 6;
    icp->startbit = (icp->buffer[0]&0x20) >> 5;
    icp->endbit = (icp->buffer[0]&0x10) >> 4;
    icp->ackbit = (icp->buffer[0]&0x08) >> 3;
    icp->cackbit = (icp->buffer[0]&0x04) >> 2;
	icp->kalive = (icp->buffer[0]&0x02) >> 1;
    ptr = (unsigned char *)&icp->size;
    *(ptr + whichEndian()) = icp->buffer[2];
    *(ptr + 1 - whichEndian()) = icp->buffer[3];
    ptr = (unsigned char *)&icp->seq;
    *(ptr + whichEndian()) = icp->buffer[4];
    *(ptr + 1 - whichEndian()) = icp->buffer[5];
    ptr = (unsigned char *)&icp->ack;
    *(ptr + whichEndian()) = icp->buffer[6];
    *(ptr + 1 - whichEndian()) = icp->buffer[7];
}

