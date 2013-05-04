#ifndef SERVER_HH
#define SERVER_HH
#include<iostream>
#include<cstdlib>
#include<cstring>
#include<cerrno>
#include<list>
#include<iterator>
#include<vector>
#include<utility>
#include<algorithm>
#include<queue>
#include<map>

#include<unistd.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#define vv
#define DEBUG

#include"helpers.hh"
#include"comm.hh"
#include"sensormsg.hh"

#define SOCK_PATH "echo_socket"

#endif
