/*
Process to handle binary protocol,reliabilty and sequencing 
*/

#include<stdio.h>
#include<sys/epoll.h>
#include"icp.h"
#include"msgqueue.h"
#include"network.h"

#define MAX_FD 100

int Epoll_Ctl(int event_fd,int option,int fd){
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = fd;
	if(epoll_ctl(event_fd,option,fd,&ev) == -1){
		perror("Error: epoll_ctl,");
		return -1;	
	}
	return 0;
}


int main(){
    int msqid;
	int ev_fd;
	if( (ev_fd = epoll_create(MAX_FD)) == -1){
		perror("Error: epoll_create,");
		return -1;
	}
	if( (msqid = setup()) == -1)
		return -1;
	if( Epoll_Ctl(ev_fd, EPOLL_CTL_ADD, msqid) == -1)
		return -1;
	while(1){
	
	}
	if( Epoll_Ctl(ev_fd, EPOLL_CTL_DEL,msqid) == -1)
		return -1;
	if(teardown(msqid) == -1)
		return -1;
	return 0;
}
