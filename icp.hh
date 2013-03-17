#ifndef IPC_HH
#define IPC_HH
#include<cstring>
#include<iostream>
int whichEndian();

class ICP{
private:
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
	
public:
	ICP();
	ICP(unsigned char buff[]);
	void toBinary();
	void toValues();
	void getBinary(unsigned char buff[]);
	void showValues();
};

#endif
