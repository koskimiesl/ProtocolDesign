#ifndef IPC_H
#define IPC_H
#include"list.h"

struct icp_struct {
    char version;           /* Version: 2 bits */
    char startbit;          /* SB: 1 bit */ 
    char endbit;            /* EB: 1 bit */
    char ackbit;            /* AB: 1 bit */
    char cackbit;           /* CAB: 1 bit */
    unsigned short size;    /* Size: 16 bits */
    unsigned short seq;     /* Sequence: 16 bits */
    unsigned short ack;     /* Acknowledgement: 16 bits */
};

struct connection {
    struct list_head list;
    char id[10];
};

int whichEndian();
void to_binary(char [],struct icp_struct *);
void to_struct(char [],struct icp_struct *);

#endif
