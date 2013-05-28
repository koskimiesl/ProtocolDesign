#ifndef IPC_H
#define IPC_H
#define DEBUG

#include<stdio.h>
int whichEndian();

struct ICP {
    unsigned char version;           /* Version: 2 bits */
    unsigned char startbit;          /* SB: 1 bit */ 
    unsigned char endbit;            /* EB: 1 bit */
    unsigned char ackbit;            /* AB: 1 bit */
    unsigned char cackbit;           /* CAB: 1 bit */
	unsigned char kalive;     		 /* KAB: 1 bit */
	unsigned char frag;				 /* FB: 1 bit */
    unsigned short size;    		 /* Size: 16 bits */
    unsigned short seq;     		 /* Sequence: 16 bits */
    unsigned short ack;     		 /* Acknowledgement: 16 bits */
	//Binary
	unsigned char buffer[8];
};
void toBinary(struct ICP * icp);
void toValues(struct ICP * icp);
inline void updateICP(struct ICP * icp,unsigned char sb,unsigned char eb,unsigned char ab,
						unsigned char cab,unsigned char ka,unsigned char fr,unsigned short s,
							unsigned short seq,unsigned short ack);
void printICPIn(struct ICP * icp);
void printICPOut(struct ICP * icp);
#endif
