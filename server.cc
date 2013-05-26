/* IoTPS Server */

#include <algorithm>
#include <arpa/inet.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "comm.hh"
#include "helpers.hh"
#include "logging.hh"
#include "sensormsg.hh"
#include "server.hh"


int main(int argc, char *argv[])
{
	// parse command line options
	char pport[PORTLEN], sport[PORTLEN]; // publish port, subscribe port
	if (getServerCmdLOpts(argc, argv, pport, sport, PORTLEN) == -1)
	{
		std::cerr << "failed to get command line options" << std::endl;
		return -1;
	}

	// start server binary protocol process
	pid_t pid;
	if ((pid = fork()) < 0)
	{
		error("fork");
		return -1;
	}
	else if (pid == 0)
	{
		if (execl("serverb", sport, (void*)0) == -1)
		{
			error("execl");
			return -1;
		}
	}

	// create publish socket
	int pfd;
	if ((pfd = custom_socket(AF_INET, pport)) == -1)
	{
		error("create publish socket");
		return -1;
	}

	// create subscribe socket (UNIX domain)
	int sfd;
	if ((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		error("create subscribe socket");
		return -1;
	}

	// bind and listen subscribe socket
	if (bindAndListenUnixS(sfd, SOCKPATH) == -1)
	{
		std::cerr << "failed to bind and listen UNIX socket" << std::endl;
		return -1;
	}

	fd_set rfds;
	std::vector<int> fds;
	fds.push_back(pfd);
	fds.push_back(sfd);
	int maxfd;
	int ret;
	struct timespec t;
	t.tv_sec = 1;
	t.tv_nsec = 0;
	size_t rsize;
	unsigned char buff[SBUFFSIZE];
	struct sockaddr addr;
	socklen_t len;
	len = sizeof(struct sockaddr);
	std::vector<std::string> sensors; // list of active sensors (device IDs)
	std::map< std::string,std::vector<int> > sublists; // device IDs mapped to fd lists (subscribers)
	CommMessage text;
	unsigned char obuff[SBUFFSIZE];
	std::string str;
	std::string cmd;
	
	#ifdef vv
	std::cout << "server started" << std::endl;
	#endif

	while (1)
	{
		FD_ZERO(&rfds); // clear file descriptors from the set
		for (size_t c = 0; c < fds.size(); c++)
			FD_SET(fds[c], &rfds); // add file descriptor to the set
		std::sort(fds.begin(), fds.end());
		maxfd = fds[fds.size()-1] + 1; // check here
		if ((ret = pselect(maxfd+1, &rfds, NULL, NULL, &t, NULL)) == -1)
		{
			error("pselect");
			continue;
		}
		for (size_t c = 0; c < fds.size(); c++)
		{
			if (FD_ISSET(fds[c], &rfds))
			{
				if (fds[c] == pfd) // message from sensor
				{
					if ((rsize = recvfrom(pfd, (char*)buff, SBUFFSIZE, 0, &addr, &len)) == -1)
					{
						error("recvfrom");
						continue;
					}
					else
					{
						double ts = getTimeStamp();
						std::string msg((char*)buff, rsize);
						SensorMessage sensormsg = SensorMessage(msg, ts);
						if (!sensormsg.parse())
						{
							std::cerr << "sensor message parsing failed" << std::endl;
							continue;
						}
						else // message successfully received
						{
							#ifdef vv
							//sensormsg.printValues();
							#endif

							// log message
							if (sensormsg.sensortype == CAMERA)
								logIncomingCamData(sensormsg, "server");
							else
								logIncomingData(sensormsg, "server");

							if (std::find(sensors.begin(), sensors.end(), sensormsg.deviceid) == sensors.end()) // new sensor
								sensors.push_back(sensormsg.deviceid);
							else // old sensor
							{
								std::vector<int> subs = sublists.find(sensormsg.deviceid)->second; // subscribers to this sensor
								#ifdef vv
								std::cout << "Number of subs to " << sensormsg.deviceid << ": " << subs.size() << std::endl;
								#endif
								for (std::vector<int>::const_iterator it = subs.begin(); it != subs.end(); it++)
								{
									text.updateServerID("server334");
									text.updateCount(1);
									text.updateSize(sensormsg.datasize);
									std::vector<std::string> tt;
									tt.push_back(sensormsg.deviceid);
									text.updateDeviceIDs(tt);
									text.updateTimeStamp(sensormsg.sensorts);
									str = text.createUpdatesMessage();
									memset(obuff, 0, SBUFFSIZE); // clear previous messages
									memcpy((char*)obuff, str.c_str(), str.size());
									if (sensormsg.sensortype == CAMERA)
									{
										if (sensormsg.sensordata == "NO_MOTION")
										{
											memcpy((char*)obuff + str.size(), sensormsg.sensordata.c_str(), sensormsg.datasize);
											double ts = getTimeStamp();
											logOutgoingData(sensormsg.deviceid, (char*)obuff + str.size(), sensormsg.datasize, ts);
										}
										else // append binary data
										{
											unsigned char camdata[sensormsg.datasize];
											sensormsg.camDataToArray(camdata);
											memcpy((char*)obuff + str.size(), camdata, sensormsg.datasize);
											logOutgoingCamData(sensormsg.deviceid, (char*)obuff + str.size(), sensormsg.datasize, ts);
										}
									}
									else
									{
										memcpy((char*)obuff + str.size(), sensormsg.sensordata.c_str(), sensormsg.datasize);
										double ts = getTimeStamp();
										logOutgoingData(sensormsg.deviceid, (char*)obuff + str.size(), sensormsg.datasize, ts);
									}
									send((*it), (char*)obuff, sensormsg.datasize + str.size(), 0);
								}
							}
						}
					}
				}
				else if (fds[c] == sfd) // connection from new client
					fds.push_back(accept(sfd, NULL, NULL));
				else // data from existing client
				{
					rsize = recv(fds[c], (char *)buff, SBUFFSIZE, 0);
					text.updateMessage((char*)buff);
					#ifdef vv
					text.print();
					#endif
					text.parse();
					cmd = text.getCommand();
					if (cmd == "LIST")
					{
						#ifdef vv
						std::cout << "LIST message" << std::endl;
						#endif
						text.updateServerID("server334");
						text.updateDeviceIDs(sensors);
						str = text.createListReply();
						memcpy((char *)buff, str.c_str(), str.size());
						send(fds[c], (char *)buff, str.size(), 0);
					}
					else if (cmd == "SUBSCRIBE")
					{
						#ifdef vv
						std::cout << "SUBSCRIBE message" << std::endl;
						#endif
						std::vector<std::string> devsToSub = text.getDeviceIDs(); // devices to subscribe
						std::vector<std::string>::const_iterator it;
						for (it = devsToSub.begin(); it != devsToSub.end(); it++)
						{
							if (std::find(sensors.begin(), sensors.end(), *it) == sensors.end())
							{
								std::cerr << "Invalid device ID in SUBSCRIBE message" << std::endl;
								continue;
							}
							std::map< std::string, std::vector<int> >::iterator itr;
							if ((itr = sublists.find(*it)) == sublists.end()) // no subscribers to this sensor
							{
								#ifdef vv
								std::cout << "No previous subscribers" << std::endl;
								#endif
								std::vector<int> clients; // create new list of clients subscribed to this sensor
								clients.push_back(fds[c]);
								sublists.insert(std::pair< std::string, std::vector<int> >(*it, clients));
							}
							else // sensor has subscribers
							{
								#ifdef vv
								std::cout << "Sensor has subscribers" << std::endl;
								#endif
								if (std::find(itr->second.begin(), itr->second.end(), fds[c]) == itr->second.end())
								{
									itr->second.push_back(fds[c]);
								}
								else // client has already subscribed to this sensor
								{
									std::cerr << "Client has already subscribed to this sensor" << std::endl;
									continue;
								}
							}
						}
						text.updateServerID("server334");
						str = text.createSubscribeReply();
						memcpy((char *)buff, str.c_str(), str.size());
						send(fds[c], (char *)buff, str.size(), 0);
					}
					else if(cmd == "UNSUBSCRIBE")
					{
						// TODO
						text.updateServerID("server334");
						str = text.createUnsubscribeReply();
						memcpy((char *)buff, str.c_str(), str.size());
						send(fds[c], (char *)buff, str.size(), 0);
					}
				}
			}
		}
	}
	return 0;
}
