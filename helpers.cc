#include <iostream>
#include <cerrno>
#include <cstring>
#include <netdb.h>

void error(std::string msg)
{
	std::cerr << msg << ":" << strerror(errno) << std::endl;
}

int customServerSocket(int family, const char port[])
{
	struct addrinfo hints, *result, *rp;
	int sfd; //status
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	if ((sfd = getaddrinfo(NULL, port, &hints, &result)) != 0)
	{
		std::cerr << "getaddrinfo: " << gai_strerror(sfd) << std::endl;
		sfd = -1;
		return sfd;
	}
	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		if ((sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1)
		{
			error("socket");
			continue;
		}
		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == -1)
		{
			error("bind");
			close(sfd);
		}
		else
			break;
	}
	if (rp == NULL)
	{
		std::cerr << "Error,could not create socket" << std::endl;
		sfd = -1;
	}
	freeaddrinfo(result);
	return sfd;
}
