/* General helper functions */
#ifndef HELPERS_H
#define HELPERS_H

#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>

#define BUFF_SIZE 65535
#define PORTLEN 10
//#define vv

int custom_socket(int ,const char []);
int custom_socket_remote(const char [],const char [],struct sockaddr *);

#endif
