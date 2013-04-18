#ifndef IPC_HH
#define IPC_HH
#include<cstring>
#include<iostream>
#define DEBUG
int whichEndian();

class ICP{
	
public:
	ICP();
	ICP(unsigned short s);
	void toValues();
	void toBinary();
	void showValues();

	//Individual
    unsigned char version;           /* Version: 2 bits */
    unsigned char startbit;          /* SB: 1 bit */ 
    unsigned char endbit;            /* EB: 1 bit */
    unsigned char ackbit;            /* AB: 1 bit */
    unsigned char cackbit;           /* CAB: 1 bit */
    unsigned short size;    		 /* Size: 16 bits */
    unsigned short seq;     		 /* Sequence: 16 bits */
    unsigned short ack;     		 /* Acknowledgement: 16 bits */
	//Binary
	unsigned char buffer[8];

};

#endif
