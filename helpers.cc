#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>

#include "helpers.hh"


int bindAndListenUnixS(int fd, const std::string sockpath)
{
	struct sockaddr_un local;
	memset(&local, 0, sizeof(struct sockaddr_un));
	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, sockpath.c_str());
	unlink(local.sun_path); // remove the file if it already exists
	int length;
	length = sizeof(local.sun_family) + strlen(local.sun_path);
	if ((bind(fd, (struct sockaddr*)&local, length)) == -1)
	{
		error("bind");
		return -1;
	}
	if ((listen(fd, 5)) == -1)
	{
		error("listen");
		return -1;
	}
	return 0;
}

int createDir(const std::string path)
{
	struct stat st;
	if (stat(path.c_str(), &st) == 0) // path exists
		return 0;
	if (mkdir(path.c_str(), 0777) == -1)
	{
		error("create directory");
		return -1;
	}
	return 0;
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
		std::cerr<<"custom_socket: getaddrinfo: "<<gai_strerror(sfd)<<std::endl;
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

void error(std::string msg)
{
	std::cerr << msg << ":" << strerror(errno) << std::endl;
}

int getServerCmdLOpts(int argc, char** argv, char* pport, char* sport, size_t portlen)
{
	int n;
	char opt;
	while ((opt = getopt(argc, argv, "s:p:")) != -1)
	{
		switch (opt)
		{
			case 's':
				strncpy(sport, optarg, PORTLEN);
				break;
			case 'p':
				strncpy(pport, optarg, PORTLEN);
				break;
			case '?':
				std::cerr << "failed to get command line option" << std::endl;
				return -1;
			default:
				std::cerr << "failed to get command line option" << std::endl;
				return -1;
		}
		n++;
	}
	if (n != 2)
	{
		std::cerr << "not enough command line options given" << std::endl;
		return -1;
	}
	return 0;
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

int setnget(int fd){
	int a = 65535;
	socklen_t s;
	s = sizeof(int);
	if(setsockopt(fd,SOL_SOCKET,SO_SNDBUF,&a,s) == -1)
		return -1;
	if(getsockopt(fd,SOL_SOCKET,SO_SNDBUF,&a,&s) == -1)
		return -1;
	return a;
}
