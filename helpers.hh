/* General helper functions */
#ifndef HELPERS_HH
#define HELPERS_HH

#include <algorithm>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>

#define BUFF_SIZE 65535
#define PORTLEN 10

int bindAndListenUnixS(int fd, const std::string sockpath);
int createDir(const std::string path);
int custom_socket(int ,const char []);
int custom_socket_remote(const char [],const char [],struct sockaddr *);
void error(std::string msg);
int getServerCmdLOpts(int argc, char** argv, char* pport, char* sport, size_t portlen);
double getTimeStamp();
int setngetS(int fd);
int setngetR(int fd);
#endif
