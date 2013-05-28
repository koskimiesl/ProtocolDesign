#ifndef CLIENTB_H
#define CLIENTB_H

#include<sys/socket.h>
#include<sys/types.h>
#include<sys/signal.h>
#include<sys/epoll.h>
#include<sys/un.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>

#include"help.h"
#include"state.h"
#include"icp.h"
#define SOCK_PATH "echo_client"

#endif
