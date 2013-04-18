#include"server.hh"

// list of all connections from clients //make it pair
std::list<State> states;
std::map< std::string,State > tuple;

// list of clients and sensors subscribed to
std::map< std::string,std::vector<std::string> > clients;
// list of sensors and subscribed clients
std::map< std::string,std::vector<std::string> > sensors;

// Dummy function returning list of available sensors
std::vector<std::string> getSensorsList(){
	std::vector<std::string> sensorslist;
	sensorslist.push_back("GPS001");
	sensorslist.push_back("Cam002");
	sensorslist.push_back("Tem003");
	sensorslist.push_back("SWT004");
	return sensorslist;
}

// Append subscribed sensors to client
void appendSubsSensorstoClient(std::string c,std::vector<std::string> s){
	std::vector<std::string> t;	
	if(clients.find(c) == clients.end()){
		// first subscribtion request
		clients.insert(std::pair< std::string,std::vector<std::string> >(c,s));
	}
	else{
		// more subscribtion request,assuming no subscribtion for previously subscribed sensors
		t = clients.find(c)->second;
		std::copy(s.begin(),s.end(),t.end());
		clients.erase(c);
		clients.insert(std::pair< std::string,std::vector<std::string> >(c,t));
	}
	// just for now
}

// Remove unsubscribed sensors from client
void removeUnSubsSensorsfromClient(std::string c,std::vector<std::string> s){
	//TODO
}

int main(int argc,char *argv[]){
	char pport[PORTLEN],sport[PORTLEN];
	unsigned char buff[BUFF_SIZE];	
	int sfd,opt; //socket fd
	socklen_t len;
	size_t rsize;
	size_t buff_size;
	std::string cmd;		
	std::string misc;
	int ret;

	ICP icp;	
	CommMessage text;
	struct sockaddr addr;
	len = sizeof(addr);
	
	std::list<State>::iterator itr;
	// Parse command line options
	while( (opt = getopt(argc,argv, "s:p:")) != -1){
		switch(opt){
			case 's':
				strncpy(sport,optarg,PORTLEN);
				#ifdef vv
				std::cout<<"Subscribe Port: "<<sport<<std::endl;
				#endif
				break;
			case 'p':
				strncpy(pport,optarg,PORTLEN);
				#ifdef vv
				std::cout<<"Publish Port: "<<pport<<std::endl;
				#endif
				break;
			case '?':
				return -1;
			default:
				return -1;		
		}
	}
	
	pthread_t thread;

	// Starting publish thread
	if( (ret = pthread_create(&thread, NULL, publishServer, (void *)&pport)) != 0)
	{
		#ifdef vv
		std::cerr<<"Unable to create thread."<<std::endl;
		#endif		
		return -1;
	}
	std::cout<<"Server started ..."<<std::endl;
	// Continue with subscribe thread
	
	if( (sfd = custom_socket(AF_INET,sport)) == -1) //AF_INET,AF_INET6 or AF_UNSPEC
		return -1;
	
	std::cout<<"Server started ..."<<std::endl;		
	while(1){
		if( (rsize = recvfrom(sfd,(char*)buff,BUFF_SIZE,0,&addr,&len)) == -1){
			error("recvfrom");		
			continue;
		}
		else if(rsize >= 8){
			#ifdef vv
			std::cout<<"Packets"<<std::endl;
			#endif
			memcpy(icp.buffer,buff,8);
			icp.toValues();
			buff_size = 0;
			if(icp.startbit == 0x01){
				#ifdef vv
				std::cout<<"Connect"<<std::endl;
				#endif
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
				buff_size = 8;
				memcpy(buff,icp.buffer,buff_size);
			}
			else if(icp.endbit == 0x01){
				#ifdef vv
				std::cout<<"Close"<<std::endl;
				#endif			
			}
			else if(icp.size != 0){
				#ifdef vv
				std::cout<<"Application data"<<std::endl;
				#endif
				// Obtaing "State" from list states
				for (itr = states.begin();itr != states.end();itr++){
					if(itr->isEqual(&addr)){
						std::cout<<"Found"<<std::endl;					
						break;	
					}		
				}
				// Update state
				(*itr).ack = icp.seq;
				// Text data			
				memmove(buff,buff+8,icp.size);
				memset(buff+icp.size,0,1000-icp.size);
				text.updateMessage((char*)buff);
				text.parse();
				#ifdef vv
				text.print();
				#endif
				cmd = text.getCommand();
				if(cmd == "LIST"){
					// Create text message
					text.updateServerID("server334");
					text.updateDeviceIDs(getSensorsList());
					misc = text.createListReply();
					// Create binary message
					(*itr).seq++;
					icp.startbit = 0x00;
					icp.endbit = 0x00;
					icp.ackbit = 0x01;
					icp.cackbit = 0x01;
					icp.size = misc.size();
					icp.seq = (*itr).seq;
					icp.ack = (*itr).ack;
					icp.toBinary();
					buff_size = 8 + icp.size;
					// copy to buffer
					memcpy(buff,icp.buffer,8);
					memcpy(buff+8,misc.c_str(),icp.size);
				}
				else if(cmd == "SUBSCRIBE"){
					// make mapping of this clent to state (connection)
					tuple.insert(std::pair<std::string,State>(text.getClientID(),*itr));
					appendSubsSensorstoClient(text.getClientID(),text.getDeviceIDs());					
					
				}
			}
			if(buff_size != 0)		
				sendto(sfd,(char*)buff,buff_size,0,&addr,len);					
		}
		else{
			// error,packets must be atleast 8 bytes
			std::cout<<"Invalid UDP packet."<<std::endl;
		}
	}
			
	return 0;
}
