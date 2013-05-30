#ifndef CLIENT_H
#define CLIENT_H

#include<iostream>
#include<cstdlib>
#include<cstring>
#include<queue>
#include<string>

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

#include"comm.hh"
#include"screen.hh"
#include"logging.hh"

#define IDLEN 10
#define IPLEN 20
#define LOSSLEN 10
#define PORTLEN 10
#define SOCK_PATH "echo_client"

enum REQ {
	LIST,
	SUBS,
	UNSUBS,
	NONE,
};

#endif

