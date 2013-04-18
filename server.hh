#ifndef SERVER_HH
#define SERVER_HH
#include<iostream>
#include<cstdlib>
#include<cstring>
#include<cerrno>
#include<list>
#include<map>
#include<iterator>
#include<vector>
#include<utility>
#include<algorithm>

#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<pthread.h>

#include"icp.hh"
#include"state.hh"
#include"helpers.hh"
#include"comm.hh"
#include"sensormsg.hh"

#define vv

// list of all connections from clients
extern std::list<State> states;
// list of clients and sensors subscribed to
extern std::map< std::string,std::vector<std::string> > clients;
// list of sensors and subscribed clients
extern std::map< std::string,std::vector<std::string> > sensors;


void * publishServer(void * arg);

#endif
