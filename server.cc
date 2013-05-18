/* IoTPS Server */
#include"server.hh"

int main(int argc,char *argv[]){
	std::vector<std::string> sensors;
	char pport[PORTLEN],sport[PORTLEN];
	unsigned char buff[BUFF_SIZE];
	unsigned char obuff[BUFF_SIZE];
	socklen_t len;	
	int opt;
	int ret;
	int pfd,sfd;
	int temp;
	int maxfd;	
	int length;	
	size_t rsize;	
	pid_t pid;
	fd_set rfds;	
	std::vector<int> fds;	
	std::string str;
	CommMessage text;
	std::string cmd;
	std::map< int,std::vector<std::string> > subs;
	std::map< std::string,std::vector<int> > list;
	struct sockaddr addr;
	struct sockaddr_un local;	
	struct timespec t;
	t.tv_sec = 1;
	t.tv_nsec = 0;
	len = sizeof(struct sockaddr);	

	// Parse command line options
	while( (opt = getopt(argc,argv, "s:p:")) != -1){
		switch(opt){
			case 's':
				strncpy(sport,optarg,PORTLEN);
				break;
			case 'p':
				strncpy(pport,optarg,PORTLEN);
				break;
			case '?':
				return -1;
			default:
				return -1;		
		}
	}

	if( (pid = fork()) < 0){
		perror("fork");
		return -1;
	}
	else if(pid == 0){
		if(execl("serverb",sport,(void *)0) == -1){
			perror("execl");
			return -1;		
		}
	}

	// create publish socket
	if ((pfd = custom_socket(AF_INET,pport)) == -1) // AF_INET, AF_INET6 or AF_UNSPEC
		return -1;

	memset(&local,0,sizeof(struct sockaddr_un));
		
	if( (sfd = socket(AF_UNIX,SOCK_STREAM,0)) == -1)
		perror("socket");
	
	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH);
	unlink(local.sun_path);
	length = strlen(local.sun_path) + sizeof(local.sun_family);

	if( (bind(sfd,(struct sockaddr *)&local,length)) == -1)
		perror("bind");
	if( (listen(sfd,5)) == -1)
		return -1;

	#ifdef vv
	std::cout<<"Server started."<<std::endl;
	#endif

	fds.push_back(pfd);
	fds.push_back(sfd);
	while(1){
		FD_ZERO(&rfds);	
		for(size_t c = 0;c < fds.size();c++)
			FD_SET(fds[c],&rfds);
		std::sort(fds.begin(),fds.end());
		maxfd = fds[fds.size()-1] + 1; //check here
		// data from sensors
		if( (ret = pselect(maxfd+1,&rfds,NULL,NULL,&t,NULL)) == -1){
			perror("pselect");
			continue;
		}
		for(size_t c = 0;c < fds.size();c++){
			if(FD_ISSET(fds[c],&rfds)){
				if(fds[c] == pfd){
					if ((rsize = recvfrom(pfd, (char*)buff, BUFF_SIZE, 0, &addr, &len)) == -1)
						continue;
					else{
						double ts = getTimeStamp();
						std::string msg((char*)buff, rsize);
						SensorMessage sensormsg = SensorMessage(msg, ts);
						if (!sensormsg.parse()){
							std::cout << "Parsing failed" << std::endl;
							continue;
						}
						else{
							sensormsg.printValues();

							if (sensormsg.sensortype == CAMERA)
								logCamSensorData(sensormsg);
							else
								logSensorData(sensormsg);

							if(std::find(sensors.begin(),sensors.end(),sensormsg.deviceid) == sensors.end()){
								sensors.push_back(sensormsg.deviceid);
							}
							else {
								std::vector<int> t = list.find(sensormsg.deviceid)->second;
								std::cout<<t.size()<<std::endl;
								for(std::vector<int>::iterator itr = t.begin();itr != t.end();itr++){
									text.updateServerID("server334");
									text.updateCount(1);
									text.updateSize(rsize);
									std::vector<std::string> tt;
									tt.push_back(sensormsg.deviceid);
									text.updateDeviceIDs(tt);
									str = text.createUpdatesMessage();
									std::cout<<str<<std::endl;
									memcpy((char *)obuff,str.c_str(),str.size());
									memcpy((char *)obuff + str.size(),buff,rsize);				
									send((*itr),(char*)obuff,rsize+str.size(),0);
									std::cout<<obuff<<std::endl;
								}
							}
						}					
					}
				}
				else if(fds[c] == sfd){
					temp = accept(sfd,NULL,NULL);	
					fds.push_back(temp);
				}
				else {
					rsize = recv(fds[c],(char *)buff,BUFF_SIZE,0);
					text.updateMessage((char*)buff);
					text.print();
					text.parse();
					cmd = text.getCommand();
					if(cmd == "LIST"){
						// Create text message
						text.updateServerID("server334");
						text.updateDeviceIDs(sensors);
						str = text.createListReply();
						memcpy((char *)buff,str.c_str(),str.size());
						send(fds[c],(char *)buff,str.size(),0);					
					}		
					else if(cmd == "SUBSCRIBE"){
						//
						subs.insert(std::pair< int,std::vector<std::string> >(fds[c],text.getDeviceIDs()));
						// 	
						#ifdef vv
						std::cout<<"Subs"<<std::endl;
						#endif
						std::vector<int> ti;
						ti.push_back(fds[c]);
						std::cout<<ti.size()<<std::endl;					
						std::vector<std::string> ts;
						ts = text.getDeviceIDs();
						std::cout<<ts.size()<<std::endl;
						for(std::vector<std::string>::iterator itr = ts.begin();itr != ts.end();itr++){
							std::cout<<*itr<<std::endl;
							list.insert(std::pair< std::string,std::vector<int> > (*itr,ti));
						} 
						text.updateServerID("server334");
						str = text.createSubscribeReply();
						memcpy((char *)buff,str.c_str(),str.size());
						send(fds[c],(char *)buff,str.size(),0);							
					}
					else if(cmd == "UNSUBSCRBE"){
						subs.erase(fds[c]);
						text.updateServerID("server334");
						str = text.createUnsubscribeReply();
						memcpy((char *)buff,str.c_str(),str.size());
						send(fds[c],(char *)buff,str.size(),0);	
					}
				}
			}		
		}			
	}
		
	return 0;
}
