#include"server.hh"

// we may need r/w lock to sync r/w between two threads 
// stores all information about connections
std::vector<State> states;

// handles binary protocol
void * lower(void * arg){
	unsigned char buff[BUFF_SIZE];	
	struct sockaddr addr;	
	ICP icp;	
	size_t rsize;
	fd_set rfds;	
	int sfd;
	int ret;	
	socklen_t len;
	struct timespec t;
	t.tv_sec = 1;
	t.tv_nsec = 0;
	len = sizeof(addr);

	std::vector<State>::iterator itr;


	if( (sfd = custom_socket(AF_INET,(char *)arg)) == -1) //AF_INET,AF_INET6 or AF_UNSPEC
		pthread_exit(NULL);

	while(1){
		FD_ZERO(&rfds);
		FD_SET(sfd,&rfds);
		if( (ret = pselect(sfd+1,&rfds,NULL,NULL,&t,NULL)) == -1){
			continue;
		}
		if(FD_ISSET(sfd,&rfds)){
			if( (rsize = recvfrom(sfd,(char*)buff,BUFF_SIZE,0,&addr,&len)) == -1){
				continue;
			}
			else if(rsize >= 8){
				memcpy(icp.buffer,buff,8);
				icp.toValues();
				if(icp.startbit == 0x01){								
					State state;
					state.status = CT;
					state.ack = icp.seq;
					memcpy(&state.addr,&addr,len);
					state.len = len;
					states.push_back(state);
					// Update
					icp.startbit = 0x01;
					icp.endbit = 0x00;
					icp.ackbit = 0x01;
					icp.cackbit = 0x01;
					icp.size = 0x0000;
					icp.seq = state.seq;
					icp.ack = state.ack;
					icp.toBinary();		
					memcpy(buff,icp.buffer,8);
					// send packet
					sendto(sfd,(char*)buff,8,0,&addr,len);		
				}
				else if(icp.endbit == 0x01){
					
				}
				else if(icp.size != 0){
					for (itr = states.begin();itr != states.end();itr++){
						if(itr->isEqual(&addr)){				
							break;	
						}		
					}	
					// Update state
					(*itr).ack = icp.seq;
					(*itr).seq++;
					memmove(buff,buff+8,icp.size);
					memset(buff+icp.size,0,1000-icp.size);	
					std::string str((char*)buff);		
					(*itr).incoming.push(str);
				}	
			}
		}
		for (itr = states.begin();itr != states.end();itr++){
			while((*itr).outgoing.size()){
				std::string str = (*itr).outgoing.front();
				(*itr).outgoing.pop();
				icp.size = str.size();
				icp.startbit = 0x00;
				icp.endbit == 0x00;
				icp.ackbit = 0x01; //read from state
				icp.cackbit = 0x01; //read from state
				icp.seq = (*itr).seq;
				icp.ack = (*itr).ack;
				icp.toBinary();
				// copy to buffer
				memcpy(buff,icp.buffer,8);
				memcpy(buff+8,str.c_str(),icp.size);
				// send packet
				sendto(sfd,(char*)buff,8+icp.size,0,&(*itr).addr,(*itr).len);			
			}	
		}	
	}
}

// main thread,handles server (publish and subscribe)
int main(int argc,char *argv[]){
	char pport[PORTLEN],sport[PORTLEN];
	unsigned char buff[BUFF_SIZE];
	struct sockaddr addr;
	std::vector<std::string> sensors;
	socklen_t len;	
	int opt;
	int ret;
	int pfd;
	fd_set rfds;
	size_t rsize;
	std::string str;
	pthread_t thread;
	CommMessage text;
	std::string cmd;
	struct timespec t;
	t.tv_sec = 1;
	t.tv_nsec = 0;
	//
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

	// starting thread to handle data exchange(lower/binary)
	if( (ret = pthread_create(&thread, NULL, lower, (void *)&sport)) != 0)
		return -1;

	// publish socket
	if ((pfd = custom_socket(AF_INET,pport)) == -1) // AF_INET, AF_INET6 or AF_UNSPEC
		return -1;
	
	std::vector<State>::iterator itr;
	while(1){
		FD_ZERO(&rfds);
		FD_SET(pfd,&rfds);
		// data from sensors
		if( (ret = pselect(pfd+1,&rfds,NULL,NULL,&t,NULL)) == -1){
			continue;
		}
		if(FD_ISSET(pfd,&rfds)){
			if ((rsize = recvfrom(pfd, (char*)buff, BUFF_SIZE, 0, &addr, &len)) == -1)
			{	
				continue;
			}
			else{
				SensorMessage msg = SensorMessage((char*)buff);
				if (!msg.parse())
					continue;
				else{
					if(std::find(sensors.begin(),sensors.end(),msg.deviceid) == sensors.end()){
						sensors.push_back(msg.deviceid);
					}				
				}
			}
		}
		// data from lower/binary layer
		for (itr = states.begin();itr != states.end();itr++){
			while((*itr).incoming.size()){
				text.updateMessage((*itr).incoming.front());
				text.print();
				(*itr).incoming.pop();
				text.parse();
				cmd = text.getCommand();
				if(cmd == "LIST"){
					// Create text message
					text.updateServerID("server334");
					text.updateDeviceIDs(sensors);
					str = text.createListReply();
					(*itr).outgoing.push(str);
				}
				else if(cmd == "SUBSCRIBE"){
				}
				else if(cmd == "UNSUBSCRIBE"){
				}							
			}	
		}			
	}

	return 0;
}
