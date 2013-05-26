#ifndef CLIENT_H
#define CLIENT_H

#include<iostream>
#include<cstdlib>
#include<cstring>
#include<queue>

#include<sys/socket.h>
#include<sys/types.h>
#include<sys/signal.h>
#include<sys/un.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include<netdb.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>

#include"helpers.hh"
#include"comm.hh"
#include"screen.hh"
#include"logging.hh"

#define IPLEN 20
#define PORTLEN 10
#define SOCK_PATH "echo_client"

enum REQ {
	LIST,
	SUBS,
	UNSUBS,
	NONE,
};

#endif

