#include<iostream>
#include<cstdlib>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include"server.hh"
#include"icp.hh"
#include<string.h>

/* Returns socket. */

int custom_socket(int family,const char port[]){
	struct addrinfo hints,*result,*rp;
	int sfd; //status
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	if( getaddrinfo(NULL,port,&hints,&result) == -1){
		std::cout<<"Error,getaddrinfo"<<std::endl;	
		sfd = -1;
	}
	for(rp = result;rp != NULL;rp = rp->ai_next){
		if( (sfd = socket(rp->ai_socktype,rp->ai_family,rp->ai_protocol)) == -1){
			std::cout<<"Error,socket"<<std::endl;
			continue;
		}
		if(bind(sfd,rp->ai_addr,rp->ai_addrlen) == -1){
			std::cout<<"Error,bind"<<std::endl;
			close(sfd);		
		}
		else
			break;
	}
	if(rp == NULL){
		std::cout<<"Error,could not create socket"<<std::endl;
		sfd = -1;
	}
	
	freeaddrinfo(result);
	
	return sfd;
}

#define BUFF_SIZE 1000


int main(){
	int sfd;
	char buff[BUFF_SIZE];
	sfd = custom_socket(AF_INET,"1500"); //AF_INET,AF_INET6 or AF_UNSPEC
	size_t rsize;	
	while(1){
		rsize =recvfrom(sfd,buff,BUFF_SIZE,0,NULL,NULL);
		std::cout<<rsize<<std::endl;
		ICP icp((unsigned char*)buff);
		icp.toValues();
		icp.showValues();
	}		
	return 0;
}
