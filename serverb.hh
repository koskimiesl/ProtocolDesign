#ifndef SERVERB_H
#define SERVERB_H

#include<sys/socket.h>
#include<sys/types.h>
#include<sys/signal.h>
#include<sys/un.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include<netdb.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>

#include"helpers.hh"
#include"state.hh"
#include"icp.hh"

#define vv
#define DEBUG

#define SOCK_PATH "echo_socket"

#endif
