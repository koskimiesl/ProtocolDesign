/* General helper functions */
#ifndef HELPERS_HH
#define HELPERS_HH

#include <algorithm>
#include <iostream>
#include <cerrno>
#include <cstring>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<iostream>

#define BUFF_SIZE 10000
#define PORTLEN 10
//#define vv

// Print custom message (msg) and error message corresponding to errno to stderr
void error(std::string msg);

int custom_socket(int ,const char []);
int custom_socket_remote(const char [],const char [],struct sockaddr *);
#endif
