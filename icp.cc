#include"icp.hh"

int whichEndian(){
    unsigned short num = 0x00FF;
    char * ptr = (char *)&num;
    if(*ptr == 0x00)
        return 0;   /* Big Endian */
    else
        return 1;   /* Little Endian */
}

ICP::ICP(){
	version = 0x00;           
    startbit = 0x00;          
    endbit = 0x00;            
    ackbit = 0x00;           
    cackbit = 0x00;          
    size = 0x0000;    
    seq = 0x0000;    
    ack = 0x0000;
}
ICP::ICP(unsigned short s){
	version = 0x01;           
    startbit = 0x01;          
    endbit = 0x00;            
    ackbit = 0x00;           
    cackbit = 0x00;          
    size = 0x0000;    
    seq = s;     
    ack = 0x0000;     
}

void ICP::toBinary(){
    unsigned char * ptr;
    buffer[0] = (version << 6)|(startbit << 5)|(endbit << 4)|(ackbit << 3)|(cackbit << 2);
    buffer[1] = 0;
    ptr = (unsigned char *)&size;
    buffer[2 + whichEndian()] = *ptr;
    buffer[3 - whichEndian()] = *(ptr+1);
    ptr = (unsigned char *)&seq;
    buffer[4 + whichEndian()] = *ptr;
    buffer[5 - whichEndian()] = *(ptr+1);
    ptr = (unsigned char *)&ack;
    buffer[6 + whichEndian()] = *ptr;
    buffer[7 - whichEndian()] = *(ptr+1);   
}

void ICP::toValues(){
    unsigned char * ptr;
    version = (buffer[0]&0xC0) >> 6;
    startbit = (buffer[0]&0x20) >> 5;
    endbit = (buffer[0]&0x10) >> 4;
    ackbit = (buffer[0]&0x08) >> 3;
    cackbit = (buffer[0]&0x04) >>2;
    ptr = (unsigned char *)&size;
    *(ptr + whichEndian()) = buffer[2];
    *(ptr + 1 - whichEndian()) = buffer[3];
    ptr = (unsigned char *)&seq;
    *(ptr + whichEndian()) = buffer[4];
    *(ptr + 1 - whichEndian()) = buffer[5];
    ptr = (unsigned char *)&ack;
    *(ptr + whichEndian()) = buffer[6];
    *(ptr + 1 - whichEndian()) = buffer[7];
	#ifdef DEBUG
	showValues();
	#endif
}

void ICP::showValues(){
	std::cout<<"Ver: "<<int(version)<<" SBit: "<<int(startbit)<<" EBit: "<<int(endbit)<<" ABit: "<<int(ackbit)<<" CABit: "<<int(cackbit)<<" Size: "<<size<<" Seq: "<<seq<<" Ack: "<<ack<<std::endl;
}
