#include<iostream>
#include<cstdlib>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include"client.hh"
#include"icp.hh"
#include<string.h>

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
   
    if( getaddrinfo(ip,port, &hints, &result) == -1){
    	std::cout<<"Error,getaddrinfo"<<std::endl;
		sfd = -1;
    }
	for(rp = result;rp != NULL;rp = rp->ai_next){
		if( (sfd = socket(rp->ai_socktype,rp->ai_family,rp->ai_protocol)) == -1){
			std::cout<<"Error,socket"<<std::endl;
			continue;
		}
		else{
			memcpy(addr,rp->ai_addr,rp->ai_addrlen);
			break;
		}
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
	int sockfd;
	ICP ipc;
	ipc.showValues();
	struct sockaddr addr;
	memset(&addr,0,sizeof(struct sockaddr));
	sockfd = custom_socket("127.0.0.1","1500",&addr);// server ip,port
	unsigned char buff[8];
	ipc.toBinary();
	ipc.getBinary(buff);	
	sendto(sockfd,(char*)buff,8,0,&addr,sizeof(addr));
	return 0;
}
