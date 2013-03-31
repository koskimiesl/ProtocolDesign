#include<iostream>
#include<cstdlib>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include"server.hh"
#include"icp.hh"
#include<cstring>
#include<cerrno>
#include<list>
#include"state.hh"
#include"helpers.hh"
#include"comm.hh"
#include<unistd.h>
#include<map>
#include<iterator>

// Dummy function returning list of available sensors
std::vector<std::string> getSensorsList(){
	std::vector<std::string> sensorslist;
	sensorslist.push_back("GPS001");
	sensorslist.push_back("Cam002");
	sensorslist.push_back("Tem003");
	sensorslist.push_back("SWT004");
	return sensorslist;
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
		std::cerr<<"getaddrinfo: "<<gai_strerror(sfd)<<std::endl;
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

#define BUFF_SIZE 1000
#define PORTLEN 10
#define vv

int main(int argc,char *argv[]){
	char pport[PORTLEN],sport[PORTLEN];
	unsigned char buff[BUFF_SIZE];	
	int sfd,opt; //socket fd
	socklen_t len;
	size_t rsize;
	size_t buff_size;
	std::string cmd;		
	std::string misc;

	ICP icp;	
	CommMessage text;
	struct sockaddr addr;
	len = sizeof(addr);
	// list of all connections from clients
	std::list<State> states;
	// list of clients and subscribed sensors
	std::map< std::string,std::vector<std::string> > clients;
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
