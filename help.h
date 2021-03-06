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

enum Event{
	LOST,
	RECEIVED
};

int custom_socket(int ,const char [],const char localip[]);
int custom_socket_remote(const char [],const char [],const char localip[],struct sockaddr *);
int getNextState(int, double, double);
int setngetR(int fd);
int setngetS(int fd);
#endif
