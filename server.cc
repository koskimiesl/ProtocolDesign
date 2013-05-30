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
		return -1;

	// create directory for log files
	if (createDir(SLOGDIR) == -1)
		return -1;

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
		return -1;

	// create subscribe socket (UNIX domain)
	int sfd;
	if ((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		error("create subscribe socket");
		return -1;
	}

	// bind and listen subscribe socket
	if (bindAndListenUnixS(sfd, SOCKPATH) == -1)
		return -1;

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
	std::map< std::string, std::vector<int> > sublists; // device IDs mapped to fd lists (subscribers)
	std::map< int, std::string> clientids; // file descriptors mapped to client IDs
	CommMessage text;
	unsigned char obuff[SBUFFSIZE]; // send buffer
	std::string str;
	std::string cmd;
	int temp;
	int wr;
	int count;
	count = 0;
	#ifdef vv
	std::cout << "Server started" << std::endl;
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
							// log message
							if (sensormsg.sensortype == CAMERA)
							{
								if (sensormsg.sensordata == "NO_MOTION")
									logServerIncoming(SLOGDIR, sensormsg, false);
								else // binary data
									logServerIncoming(SLOGDIR, sensormsg, true);
							}
							else
								logServerIncoming(SLOGDIR, sensormsg, false);
							if (std::find(sensors.begin(), sensors.end(), sensormsg.deviceid) == sensors.end()) // new sensor
								sensors.push_back(sensormsg.deviceid);
							else // old sensor
							{
								if (sublists.size() == 0)
									continue;
								std::vector<int> subs = sublists.find(sensormsg.deviceid)->second; // subscribers to this sensor
								#ifdef vv
								std::cout << "number of subs to " << sensormsg.deviceid << ": " << subs.size() << std::endl;
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
									std::string clientid = clientids[*it];
									if (sensormsg.sensortype == CAMERA)
									{
										if (sensormsg.sensordata == "NO_MOTION")
										{
											memcpy((char*)obuff + str.size(), sensormsg.sensordata.c_str(), sensormsg.datasize);
											double ts = getTimeStamp();
											logServerOutgoing(SLOGDIR, clientid, sensormsg.deviceid, (char*)obuff + str.size(), sensormsg.datasize, ts, false);
										}
										else // append binary data
										{
											unsigned char camdata[sensormsg.datasize];
											sensormsg.camDataToArray(camdata);
											memcpy((char*)obuff + str.size(), camdata, sensormsg.datasize);
											logServerOutgoing(SLOGDIR, clientid, sensormsg.deviceid, (char*)obuff + str.size(), sensormsg.datasize, ts, true);
										}
									}
									else
									{
										memcpy((char*)obuff + str.size(), sensormsg.sensordata.c_str(), sensormsg.datasize);
										double ts = getTimeStamp();
										logServerOutgoing(SLOGDIR, clientid, sensormsg.deviceid, (char*)obuff + str.size(), sensormsg.datasize, ts, false);
									}
									wr = send((*it), (char*)obuff, sensormsg.datasize + str.size(), 0);
									std::cout<<"UTotal"<<count++<<std::endl;				
									std::cout<<"Size: "<<
								sensormsg.datasize+str.size()<<"\t"<<wr<<std::endl;								
								}
							}
						}
					}
				}
				else if (fds[c] == sfd) // connection from new client
				{
					temp = accept(sfd, NULL, NULL);
					fds.push_back(temp);
				}
				else // data from existing client
				{
					memset(buff, 0, SBUFFSIZE); // clear previous messages
					rsize = recv(fds[c], (char *)buff, SBUFFSIZE, 0);
					if (rsize == 0)
					{
						#ifdef vv
						std::cout << "client closed connection, cleaning client data" << std::endl;
						#endif
						std::map< std::string, std::vector<int> >::iterator it;
						for (it = sublists.begin(); it != sublists.end(); it++) // remove subscriptions for this client
						{
							std::vector<int>::iterator itr = std::find(it->second.begin(), it->second.end(), fds[c]);
							if (itr != it->second.end())
								it->second.erase(itr);
						}
						if (clientids.erase(fds[c]) != 1) // remove client ID
						{
							std::cerr << "failed to remove client ID" << std::endl;
							return -1;
						}
						if (close(fds[c]) == -1)
						{
							perror("close");
							return -1;
						}
						continue;
					}
					text.updateMessage((char*)buff);
					#ifdef vv
					text.print();
					#endif
					text.parse();
					cmd = text.getCommand();
					if (cmd == "LIST")
					{
						text.updateServerID("server334");
						text.updateDeviceIDs(sensors);
						str = text.createListReply();
						memcpy((char *)buff, str.c_str(), str.size());
						send(fds[c], (char *)buff, str.size(), 0);
					}
					else if (cmd == "SUBSCRIBE")
					{
						clientids.insert(std::pair< int, std::string>(fds[c], text.getClientID()));
						std::vector<std::string> devsToSub = text.getDeviceIDs(); // devices to subscribe
						std::vector<std::string>::iterator it;
						for (it = devsToSub.begin(); it != devsToSub.end(); it++)
						{
							if (std::find(sensors.begin(), sensors.end(), *it) == sensors.end())
							{
								std::cerr << "invalid device ID in SUBSCRIBE message" << std::endl;
								devsToSub.erase(it);
								continue;
							}
							std::map< std::string, std::vector<int> >::iterator itr;
							if ((itr = sublists.find(*it)) == sublists.end()) // no subscribers to this sensor
							{
								#ifdef vv
								std::cout << "no previous subscribers" << std::endl;
								#endif
								std::vector<int> clients; // create new list of clients subscribed to this sensor
								clients.push_back(fds[c]);
								sublists.insert(std::pair< std::string, std::vector<int> >(*it, clients));
							}
							else // sensor has subscribers
							{
								#ifdef vv
								std::cout << "sensor has subscribers" << std::endl;
								#endif
								if (std::find(itr->second.begin(), itr->second.end(), fds[c]) == itr->second.end())
								{
									itr->second.push_back(fds[c]);
								}
								else // client has already subscribed to this sensor
								{
									std::cerr << "client has already subscribed to this sensor" << std::endl;
									continue;
								}
							}
						}
						text.updateDeviceIDs(devsToSub);
						text.updateCount(devsToSub.size());
						text.updateServerID("server334");
						str = text.createSubscribeReply();
						#ifdef vv
						std::cout << str << std::endl;
						#endif
						memcpy((char *)buff, str.c_str(), str.size());
						send(fds[c], (char *)buff, str.size(), 0);
					}
					else if (cmd == "UNSUBSCRIBE")
					{
						#ifdef vv
						std::cout << "unsub request from " << text.getDeviceIDs().size() << " device(s)" << std::endl;
						#endif
						std::vector<std::string> devsToUnsub = text.getDeviceIDs(); // devices to unsubscribe from
						std::vector<std::string>::iterator it;
						for (it = devsToUnsub.begin(); it != devsToUnsub.end(); it++)
						{
							if (std::find(sensors.begin(), sensors.end(), *it) == sensors.end())
							{
								std::cerr << "invalid device ID in UNSUBSCRIBE message" << std::endl;
								devsToUnsub.erase(it);
								continue;
							}
							std::map< std::string, std::vector<int> >::iterator itr;
							if ((itr = sublists.find(*it)) == sublists.end()) // sensor has no subscribers
							{
								std::cerr << "sensor has no subscribers" << std::endl;
								devsToUnsub.erase(it);
								continue;
							}
							else // sensor has subscribers
							{
								std::vector<int>::iterator itrt = std::find(itr->second.begin(), itr->second.end(), fds[c]);
								if (itrt == itr->second.end()) // client has not subscribed to this sensor
								{
									std::cerr << "client has not subscribed to this sensor" << std::endl;
									devsToUnsub.erase(it);
									continue;
								}
								else // valid unsubscribe request
								{
									#ifdef vv
									std::cout << "valid unsubscribe request" << std::endl;
									#endif
									itr->second.erase(itrt);
									if (itr->second.size() == 0) // client is the only subscriber to this sensor
									{
										#ifdef vv
										std::cout << "client is the only subscriber to this sensor" << std::endl;
										#endif
										sublists.erase(itr);
									}
								}
							}
						}
						text.updateDeviceIDs(devsToUnsub);
						text.updateCount(devsToUnsub.size());
						text.updateServerID("server334");
						str = text.createUnsubscribeReply();
						#ifdef vv
						std::cout << str << std::endl;
						#endif
						memcpy((char *)buff, str.c_str(), str.size());
						send(fds[c], (char *)buff, str.size(), 0);
					}
				}
			}
		}
	}
	return 0;
}
