#include <iomanip>
#include <sstream>
#include <sys/time.h>
#include "helpers.hh"

void error(std::string msg)
{
	std::cerr << msg << ":" << strerror(errno) << std::endl;
}

/* Returns socket */
int custom_socket_remote(const char ip[],const char port[],struct sockaddr * addr){
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

double getTimeStamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	double timestamp = (double)tv.tv_sec + (double)1e-6 * tv.tv_usec;
	std::stringstream ss;
	ss << std::setprecision(3) << std::setiosflags(std::ios_base::fixed) << timestamp;
	ss >> timestamp;
	return timestamp;
}
