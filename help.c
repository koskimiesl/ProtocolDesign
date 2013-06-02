#include<stdlib.h>
#include<time.h>

#include"help.h"

/* Returns socket */
int custom_socket_remote(const char ip[],const char port[],const char localip[],struct sockaddr * addr){
	struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;    
    hints.ai_protocol = 0;          /* Any protocol */
   
    if( getaddrinfo(ip,port, &hints, &result) != 0){
    	perror("getaddrinfo, ");
		return -1;
    }
	for(rp = result;rp != NULL;rp = rp->ai_next){
		if( (sfd = socket(rp->ai_family,rp->ai_socktype,rp->ai_protocol)) == -1){
			perror("socket, ");
			continue;
		}
		else{
			memcpy(addr,rp->ai_addr,rp->ai_addrlen);
			break;
		}
	}
	/* None worked */
	if(rp == NULL)
		sfd = -1;

	freeaddrinfo(result);
	
	if(sfd != -1){
		/* This part works for ipv4 only */
		struct sockaddr_in laddr;
		memset(&laddr,0,sizeof(struct sockaddr_in));
		laddr.sin_family= AF_INET;
		laddr.sin_port=0;
		inet_pton(AF_INET,localip,(void*)&(laddr.sin_addr.s_addr)); 

		if(bind(sfd,(struct sockaddr*)&laddr,sizeof(struct sockaddr_in))== -1){
			perror("bind, ");			
			sfd = -1;		
		}

	}
	return sfd;
}

/* Returns socket. */
int custom_socket(int family,const char port[],const char localip[]){
	struct addrinfo hints,*result,*rp;
	int sfd;
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	if( getaddrinfo(localip,port,&hints,&result) != 0){
		perror("getaddrinfo, ");
		return -1;
	}
	for(rp = result;rp != NULL;rp = rp->ai_next){
		if( (sfd = socket(rp->ai_family,rp->ai_socktype,rp->ai_protocol)) == -1){
			perror("socket, ");
			continue;
		}
		if(bind(sfd,rp->ai_addr,rp->ai_addrlen) == -1){
			perror("bind, ");
			close(sfd);		
		}
		else
			break;
	}	
	/* None worked. */
	if(rp == NULL)
		sfd = -1;
	
	freeaddrinfo(result);
	
	return sfd;
}

// setting p == q means independent loss, as far as I understand
// otherwise loss is dependent of previous state
int getNextState(int previous, double p, double q)
{
	if (p == 0.0 && q == 0.0)
		return RECEIVED;
	
	if (!(previous == LOST || previous == RECEIVED)) // invalid previous state
		return -1;
	if (p < 0 || p > 1 || q < 0 || q > 1) // invalid loss ratio
		return -1;

	srand(time(NULL));
	if (previous == RECEIVED)
	{
		if (rand() <  p * ((double)RAND_MAX + 1.0))
			return LOST;
		else
			return RECEIVED;
	}
	else
	{
		if (rand() < q * ((double)RAND_MAX + 1.0))
			return LOST;
		else
			return RECEIVED;
	}
}

int setngetR(int fd){
	int a = 200000;	
	socklen_t s;
	s = sizeof(int);
	if(setsockopt(fd,SOL_SOCKET,SO_RCVBUF,&a,s) == -1)
		return -1;
	if(getsockopt(fd,SOL_SOCKET,SO_RCVBUF,&a,&s) == -1)
		return -1;
	return a;
}

int setngetS(int fd){
	int a = 200000;	
	socklen_t s;
	s = sizeof(int);
	if(setsockopt(fd,SOL_SOCKET,SO_SNDBUF,&a,s) == -1)
		return -1;
	if(getsockopt(fd,SOL_SOCKET,SO_SNDBUF,&a,&s) == -1)
		return -1;
	return a;
}
