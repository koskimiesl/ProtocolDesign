// declare struct that contains state information about a connection
#ifndef STATE_H
#define STATE_H
#include<sys/socket.h>
#include<sys/types.h>
#include<time.h>
#include<sys/time.h>
#include<stdlib.h>
#include<string.h>
#include"list.c"

typedef enum{
	false,
	true,
}bool;

enum Status{
	RC,
	CT,
	RE,
};

struct Int {
	struct list_head list;
	unsigned short seq;
};

struct Queue {
	struct list_head list;
	unsigned short seq;
	unsigned char frag;	
	size_t size;
	struct timeval st;
	bool sent;
	unsigned char buffer[1500];
};

struct State {
	struct list_head list;
	// remote addres
	struct sockaddr addr;
	socklen_t len;
	// packet info
	unsigned short seq;
	// sent remote
	unsigned short sentdown;
	// acked index
	unsigned short ack;
	// remote acked index 
	unsigned short rack; // cumulative
	struct timeval kt; // keepalive time
	unsigned int window; 
	struct Int racks; // individual
	// sent to app layer
	unsigned short sentup;
	bool ackreq;
	int fd;
	enum Status status;
	struct Queue out;
	struct Queue in;
};

void initState(struct State * state);
struct State * findState_addr(struct State * state,struct sockaddr * addr);
struct State * findState_fd(struct State * state,int fd);
void ackThis(struct State * state,unsigned short a,unsigned char ackbit,unsigned char cackbit);
bool ackThat(struct State * state,unsigned short a);
void addOutPacketToState(struct State * state,unsigned char * packet,unsigned short seq,
int size,unsigned char frag);
void addInPacketToState(struct State * state,unsigned char * packet,unsigned short seq,
int size,unsigned char frag);
bool checktime(struct timeval *pt,struct timeval *ct,size_t gap);
#endif
