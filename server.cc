#include<iostream>
#include<cstdlib>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include"server.hh"
#include"icp.hh"
#include<cstring>
#include<cerrno>
#include<list>
#include"state.hh"
#include"helpers.hh"
#include"comm.hh"
#include<unistd.h>

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

	if( (sfd = getaddrinfo(NULL,port,&hints,&result)) != 0){
		std::cerr<<"getaddrinfo: "<<gai_strerror(sfd)<<std::endl;
		sfd = -1;
		return sfd;
	}
	for(rp = result;rp != NULL;rp = rp->ai_next){
		if( (sfd = socket(rp->ai_family,rp->ai_socktype,rp->ai_protocol)) == -1){
			error("socket");
			continue;
		}
		if(bind(sfd,rp->ai_addr,rp->ai_addrlen) == -1){
			error("bind");
			close(sfd);		
		}
		else
			break;
	}	
	if(rp == NULL){
		std::cerr<<"Error,could not create socket"<<std::endl;
		sfd = -1;
	}
	
	freeaddrinfo(result);
	
	return sfd;
}

#define BUFF_SIZE 1000
#define PORTLEN 10
#define vv

int main(int argc,char *argv[]){
	char pport[PORTLEN],sport[PORTLEN];
	unsigned char buff[BUFF_SIZE];	
	int sfd,opt; //socket fd
	socklen_t len;
	size_t rsize;
		
	ICP icp;	
	struct sockaddr addr;
	len = sizeof(addr);
	std::list<State> states;

	// Parse command line options
	while( (opt = getopt(argc,argv, "s:p:")) != -1){
		switch(opt){
			case 's':
					strncpy(sport,optarg,PORTLEN);
					#ifdef vv
					std::cout<<"Subscribe Port: "<<sport<<std::endl;
					#endif
					break;
			case 'p':
					strncpy(pport,optarg,PORTLEN);
					#ifdef vv
					std::cout<<"Publish Port: "<<pport<<std::endl;
					#endif
					break;
			case '?':
					return -1;
			default:
					return -1;		
		}
	}
	
	if( (sfd = custom_socket(AF_INET,sport)) == -1) //AF_INET,AF_INET6 or AF_UNSPEC
		return -1;
	
	std::cout<<"Server started ..."<<std::endl;		
	while(1){
		if( (rsize = recvfrom(sfd,(char*)buff,BUFF_SIZE,0,&addr,&len)) == -1){
			error("recvfrom");		
			continue;
		}
	    else if(rsize == 8){
			#ifdef vv
			std::cout<<"Connect or Close or Keepalive."<<std::endl;
			#endif			
			// connect or close or keep-alive
			memcpy(icp.buffer,buff,8);
			icp.toValues();
			if(icp.startbit == 0x01){
				State state;
				// Update state information				
				state.status = CT;
				state.ack = icp.seq;		
				states.push_back(state);
				// Update binary protocol
				icp.startbit = 0x01;
				icp.endbit = 0x00;
				icp.ackbit = 0x01;
				icp.cackbit = 0x01;
				icp.seq = state.seq;
				icp.ack = state.ack;
				icp.toBinary();
				memcpy(buff,icp.buffer,8);
				sendto(sfd,(char*)buff,8,0,&addr,len);
			}
		}
		else if(rsize > 8){
			std::cout<<"Application data."<<std::endl;
			// application data
			memcpy(icp.buffer,buff,8);
			icp.toValues();
			memmove(buff,buff+8,icp.size);
			memset(buff+icp.size,0,1000-icp.size);
			CommMessage text((char*)buff);
			#ifdef vv
			text.print();
			#endif
			//check seq,acks etc		

		}
		else{
			// error,packets must be atleast 8 bytes
			std::cout<<"Invalid UDP packet."<<std::endl;
			continue;
		}
	}		
	return 0;
}
