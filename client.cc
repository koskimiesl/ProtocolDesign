#include<iostream>
#include<cstdlib>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include"client.hh"
#include"icp.hh"
#include<cstring>
#include"state.hh"
#include"comm.hh"
#include"helpers.hh"

#include<unistd.h>

/* Returns socket */

int custom_socket(const char ip[],const char port[],struct sockaddr * addr){
	struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;    
    hints.ai_protocol = 0;          /* Any protocol */
   
    if( (sfd = getaddrinfo(ip,port, &hints, &result)) != 0){
    	std::cerr<<"getaddrinfo: "<<gai_strerror(sfd)<<std::endl;
		sfd = -1;
		return sfd;
    }
	for(rp = result;rp != NULL;rp = rp->ai_next){
		if( (sfd = socket(rp->ai_family,rp->ai_socktype,rp->ai_protocol)) == -1){
			error("socket");
			continue;
		}
		else{
			memcpy(addr,rp->ai_addr,rp->ai_addrlen);
			break;
		}
	}
	if(rp == NULL){
		std::cerr<<"Error,could not create socket"<<std::endl;
		sfd = -1;
	}	

	freeaddrinfo(result);

	return sfd;
}

#define BUFF_SIZE 1000
#define IPLEN 20
#define PORTLEN 10
#define vv

int main(int argc,char *argv[]){
	char ip[IPLEN],port[PORTLEN];
	unsigned char buff[BUFF_SIZE];
	int sfd,opt;
	socklen_t len; // size of address structure
	size_t rsize;
	struct sockaddr d_addr; // address structure for destination
	struct sockaddr s_addr; // address structure for source
	State state; //connection state
	ICP icp(state.seq); // binary protocol class with initialized sequnce nr. 

	// Parse command line options
	while( (opt = getopt(argc,argv, "s:p:")) != -1){
		switch(opt){
			case 's':
					strncpy(ip,optarg,IPLEN);
					#ifdef vv
					std::cout<<"Server IP: "<<ip<<std::endl;
					#endif
					break;
			case 'p':
					strncpy(port,optarg,PORTLEN);
					#ifdef vv
					std::cout<<"Server Port: "<<port<<std::endl;
					#endif
					break;
			case '?':
					return -1;
			default:
					return -1;		
		}
	}

	memset(&d_addr,0,sizeof(struct sockaddr));
	if( (sfd = custom_socket(ip,port,&d_addr)) == -1)// server ip,port
		return -1;

	// Connect
	// Send empty binary packet with start bit set	
	icp.toBinary();
	memcpy(buff,icp.buffer,8);
	sendto(sfd,(char*)buff,8,0,&d_addr,sizeof(d_addr));

	len = sizeof(s_addr);
	// Wait for replies
	while(1){
		// use recvfrom with select or poll
		if( (rsize = recvfrom(sfd,(char*)buff,BUFF_SIZE,0,&s_addr,&len)) == -1){
			error("recvfrom");			
			continue;
		}
		else if(rsize == 8){
			// conenct or end or keepalive 
			memcpy(icp.buffer,buff,8);
			icp.toValues();
			if(icp.startbit == 0x01 && state.status == RC){
				// Update state information				
				state.status = CT;
				state.ack = icp.seq;		
				state.seq++;
				// Update binary protocol
				icp.startbit = 0x00;
				icp.endbit = 0x00;
				icp.ackbit = 0x01;
				icp.cackbit = 0x01;
				icp.seq = state.seq;
				icp.ack = state.ack;			
				icp.toBinary();
				memcpy(buff,icp.buffer,8);
				sendto(sfd,(char*)buff,8,0,&d_addr,sizeof(d_addr));
			}	
		}
		else if(rsize > 8){		
			// application data
		}	
		else {
			//error,packets must be atleast 8 bytes
			continue;
		}
		if(state.status == CT){
			CommMessage text;
			text.updateClientID("kjlkjlk");
			std::string msg = text.createListRequest();
			std::cout<<msg<<std::endl;
			icp.size = msg.size();
			icp.startbit = 0x00;
			icp.toBinary();
			memcpy(buff,icp.buffer,8);
			memcpy(buff+8,msg.c_str(),icp.size);
			// just for now
			state.status = RE;
			sendto(sfd,(char*)buff,8+icp.size,0,&d_addr,sizeof(d_addr));		
		}	
	}
	return 0;
}
