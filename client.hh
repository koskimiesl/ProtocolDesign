#ifndef CLIENT_H
#define CLIENT_H

#include<iostream>
#include<cstdlib>
#include<cstring>
#include<queue>

#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include<netdb.h>
#include<unistd.h>

#define IPLEN 20
#define PORTLEN 10

enum REQ {
	LIST,
	SUBS,
	UNSUBS,
	NONE,
};

struct address{
	char ip[IPLEN];
	char port[PORTLEN];	
};

#endif

