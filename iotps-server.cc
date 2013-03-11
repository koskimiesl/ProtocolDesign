/* IoT-PS Server */

#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "helpers.hh"

int main(int argc, char* argv[])
{
	int sockfd, sAddrLen, publishPort, n;
	socklen_t cAddrLen;
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	char buf[1024];

	// check command line arguments
	publishPort = getCmdOptAsInt(argv, argv + argc, "-p");
	if (argc != 3 || !publishPort)
	{
		std::cerr << "Usage: " << argv[0] << " -p <publishPort>" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(publishPort);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	sAddrLen = sizeof(struct sockaddr_in);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		error("Socket creation");
	if (bind(sockfd, (struct sockaddr*)&serverAddr, sAddrLen) < 0)
		error("Socket binding");
	cAddrLen = sizeof(struct sockaddr_in);
	while (1)
	{
		if ((n = recvfrom(sockfd, buf, 1024, 0, (struct sockaddr*)&clientAddr, &cAddrLen)) < 0)
			error("Receive data");
		std::cout << "Received " << n << " bytes from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
		std::cout << "Data: " << buf << std::endl << std::endl;
	}
	return 0;
}
