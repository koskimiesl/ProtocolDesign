#ifndef SERVER_HH
#define SERVER_HH

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <list>
#include <iterator>
#include <utility>
#include <algorithm>
#include <queue>
#include <map>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "comm.hh"
#include "sensormsg.hh"
#include "logging.hh"

#define vv
#define DEBUG
#define SBUFFSIZE 10000 // server receive/send buffer size
#define SPORTLEN 10 // server port string length
#define SOCKPATH "echo_socket"

#endif
