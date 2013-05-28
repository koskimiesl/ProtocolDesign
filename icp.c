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
    icp->buffer[0] = (icp->version << 6)|(icp->startbit << 5)|(icp->endbit << 4)|(icp->ackbit << 3)|(icp->cackbit << 2)|(icp->kalive << 1) | (icp->frag);
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
	icp->frag = (icp->buffer[0]&0x01);
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

inline void updateICP(struct ICP * icp,unsigned char sb,unsigned char eb,unsigned char ab,
						unsigned char cab,unsigned char ka,unsigned char fr,unsigned short s,
							unsigned short seq,unsigned short ack){
	icp->version = 0x01;
	icp->startbit = sb;
	icp->endbit = eb;
	icp->ackbit = ab;
	icp->cackbit = cab;
	icp->kalive = ka;
	icp->frag = fr;
	icp->size = s;
	icp->seq = seq;
	icp->ack = ack;
}

void printICPIn(struct ICP * icp){
	printf("--> %d %d %d %d %d %d\n",(int)(icp->ackbit),(int)(icp->cackbit),
			(int)(icp->frag),icp->size,icp->seq,icp->ack); 
}

void printICPOut(struct ICP * icp){
	printf("<-- %d %d %d %d %d %d\n",(int)(icp->ackbit),(int)(icp->cackbit),
			(int)(icp->frag),icp->size,icp->seq,icp->ack); 
}
