/* For testing to receive sensor messages */

#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include "helpers.hh"
#include "sensormsg.hh"

#define BUFF_SIZE 10000
#define PORTLEN 10

int main(int argc, char *argv[])
{
	char pport[PORTLEN];
	unsigned char buff[BUFF_SIZE];
	int sfd, opt; // socket file descriptor, command line option
	socklen_t len;
	size_t rsize;
	struct sockaddr addr;
	len = sizeof(addr);

	// Parse command line options
	while ((opt = getopt(argc, argv, "p:")) != -1)
	{
		switch (opt)
		{
			case 'p':
					strncpy(pport, optarg, PORTLEN);
					std::cout << "Publish Port: " << pport << std::endl;
					break;
			case '?':
					return -1;
			default:
					return -1;
		}
	}

	if ((sfd = customServerSocket(AF_INET, pport)) == -1) // AF_INET, AF_INET6 or AF_UNSPEC
		return -1;

	std::cout << "Receiver started..." << std::endl;
	size_t bytes_recvd = 0;
	while (1)
	{
		if ((rsize = recvfrom(sfd, (char*)buff, BUFF_SIZE, 0, &addr, &len)) == -1)
		{
			error("recvfrom");
			continue;
		}
		else
		{
			std::cout << "Received " << rsize << " bytes:" << std::endl;
			SensorMessage msg = SensorMessage((char*)buff);
			if (!msg.parse())
				std::cerr << "Parsing failed" << std::endl;
			else
				msg.printValues();
			memset(buff, 0, BUFF_SIZE);
			bytes_recvd += rsize;
			std::cout << "Bytes received so far: " << bytes_recvd << std::endl;
		}
	}
	return 0;
}
