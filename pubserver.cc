#include "server.hh"

void* publishServer(void* arg)
{
	int pfd;
	size_t rsize;
	socklen_t len;
	struct sockaddr addr;
	unsigned char buff[BUFF_SIZE];
	std::vector<std::string> getclients;	

	if ((pfd = custom_socket(AF_INET,(char *)arg)) == -1) // AF_INET, AF_INET6 or AF_UNSPEC
		pthread_exit(NULL);

	while (1)
	{
		if ((rsize = recvfrom(pfd, (char*)buff, BUFF_SIZE, 0, &addr, &len)) == -1)
		{
			error("recvfrom");
			continue;
		}
		else
		{
			std::cout << "Received " << rsize << " bytes" << std::endl;
			SensorMessage msg = SensorMessage((char*)buff);
			if (!msg.parse())
				std::cerr << "Parsing failed" << std::endl;
			else
			{
				#ifdef vv
				msg.printValues();
				#endif
				// local logging	
				if (sensors.find(msg.deviceid) == sensors.end())
				{
					#ifdef vv
					std::cout << "New device" << std::endl;
					#endif
					std::vector<std::string> subclients; // subscribed clients
					sensors.insert(std::pair< std::string, std::vector<std::string> >(msg.deviceid, subclients));
				}
				else
				{
					#ifdef vv
					std::cout << "Current Device" << std::endl;
					#endif
					getclients = sensors.find(msg.deviceid)->second;
					if (!getclients.size())
					{
						#ifdef vv					
						std::cout << "No clients subscribed" << std::endl;
						#endif		
					}
					else
					{
						// send sensors' data to each subscribed clients
						std::cout << "Sending it" << std::endl;
					}
				}
			}
		}
	}
	pthread_exit(arg);
}
