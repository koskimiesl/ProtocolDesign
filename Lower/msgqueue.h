#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/msg.h>
#include<errno.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

#define PATH "temp"
#define ID 'x'


struct address{
	char ip[25]; // null terminated
	int port;
};

struct msgbuf_establish{
	long mtype;
	struct address address;
};

int setup(); // Lower
int join(); //Upper
int teardown(int); // Lower
int establish(int, const char [], int); // Upper

#endif
