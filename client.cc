#include<iostream>
#include<cstdlib>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include"client.hh"
#include"icp.hh"
#include<cstring>
#include"state.hh"
#include"comm.hh"
#include"helpers.hh"
#include<sys/select.h>
#include<unistd.h>

// Displays sensors available in server 
void showList(std::vector<std::string> l){
	size_t index;
	std::vector<std::string>::iterator itr;
	index = 1;	
	std::cout<<"List of sensors."<<std::endl;
	for(itr = l.begin();itr != l.end();itr++){
		std::cout<<index++<<" "<<(*itr)<<std::endl;	
	}
}

// Displays subscribed sensors
void showSubs(std::vector<std::string> l){
	size_t index;
	std::vector<std::string>::iterator itr;
	index = 1;	
	std::cout<<"List of subscribed sensors."<<std::endl;
	for(itr = l.begin();itr != l.end();itr++){
		std::cout<<index++<<" "<<(*itr)<<std::endl;	
	}
}

/* Returns socket */

int custom_socket(const char ip[],const char port[],struct sockaddr * addr){
	struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;    
    hints.ai_protocol = 0;          /* Any protocol */
   
    if( (sfd = getaddrinfo(ip,port, &hints, &result)) != 0){
    	std::cerr<<"getaddrinfo: "<<gai_strerror(sfd)<<std::endl;
		sfd = -1;
		return sfd;
    }
	for(rp = result;rp != NULL;rp = rp->ai_next){
		if( (sfd = socket(rp->ai_family,rp->ai_socktype,rp->ai_protocol)) == -1){
			error("socket");
			continue;
		}
		else{
			memcpy(addr,rp->ai_addr,rp->ai_addrlen);
			break;
		}
	}
	if(rp == NULL){
		std::cerr<<"Error,could not create socket"<<std::endl;
		sfd = -1;
	}	

	freeaddrinfo(result);

	return sfd;
}

#define BUFF_SIZE 1000
#define IPLEN 20
#define PORTLEN 10
#define vv

int main(int argc,char *argv[]){
	char ip[IPLEN],port[PORTLEN];
	unsigned char buff[BUFF_SIZE];
	int sfd,opt;
	socklen_t len; // size of address structure
	size_t rsize;
	size_t buff_size;
	struct sockaddr d_addr; // address structure for destination
	struct sockaddr s_addr; // address structure for source
	fd_set rfds;
	int ret;
	char option;
	std::string cmd;
	std::vector<std::string> list;

	State state; //connection state
	ICP icp(state.seq); // binary protocol class with initialized sequnce nr. 
	CommMessage text;

	// Parse command line options
	while( (opt = getopt(argc,argv, "s:p:")) != -1){
		switch(opt){
			case 's':
				strncpy(ip,optarg,IPLEN);
				#ifdef vv
				std::cout<<"Server IP: "<<ip<<std::endl;
				#endif
				break;
			case 'p':
				strncpy(port,optarg,PORTLEN);
				#ifdef vv
				std::cout<<"Server Port: "<<port<<std::endl;
				#endif
				break;
			case '?':
				return -1;
			default:
				return -1;		
		}
	}

	memset(&d_addr,0,sizeof(struct sockaddr));
	if( (sfd = custom_socket(ip,port,&d_addr)) == -1)// server ip,port
		return -1;
	
	// Connect
	// Send empty binary packet with start bit set	
	icp.toBinary();
	memcpy(buff,icp.buffer,8);
	sendto(sfd,(char*)buff,8,0,&d_addr,sizeof(d_addr));
	len = sizeof(s_addr);

	// Wait for replies
	while(1){
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);
		FD_SET(sfd,&rfds);		
		if( (ret = select(sfd+1,&rfds,NULL,NULL,NULL)) == -1){
			#ifdef vv
			std::cout<<"Select,error"<<std::endl;
			#endif
			continue;
		}
		if(FD_ISSET(sfd,&rfds)){
			
			if( (rsize = recvfrom(sfd,(char*)buff,BUFF_SIZE,0,&s_addr,&len)) == -1){
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
				if(icp.startbit == 0x01 && state.status == RC){
					#ifdef vv
					std::cout<<"Connect"<<std::endl;
					#endif
					// Update state 
					state.status = CT;
					state.ack = icp.seq;
					state.seq++;
					// Update
					icp.startbit = 0x00;
					icp.endbit = 0x00;
					icp.ackbit = 0x01;
					icp.cackbit = 0x01; // logic here
					icp.seq = state.seq;
					icp.ack = state.ack;			
					icp.toBinary();
					buff_size = 8;
					memcpy(buff,icp.buffer,buff_size);
				}
				else if(icp.endbit == 0x01 && state.status == CT){
				}
				else if(icp.size != 0){
					#ifdef vv
					std::cout<<"Application data"<<std::endl;
					#endif
					// Update state
					state.ack = icp.seq;
					// Text data			
					memmove(buff,buff+8,icp.size);
					memset(buff+icp.size,0,1000-icp.size);
					text.updateMessage((char*)buff);
					text.parse();
					#ifdef vv
					text.print();
					#endif
					cmd = text.getCommand();
					if(cmd == "OK"){
						list = text.getDeviceIDs();
					}
					else if(cmd == "UPDATES"){
					}					
				}
				if(buff_size != 0)
					sendto(sfd,(char*)buff,8,0,&s_addr,len);
			}
			else{
				// error,packets must be atleast 8 bytes
				std::cout<<"Invalid UDP packet."<<std::endl;		
			}
		}
		else if(FD_ISSET(0,&rfds)){
			#ifdef vv
			std::cout<<"User input"<<std::endl;
			#endif		
			std::cin>>option;
			// sends LIST request
			if(option == 'g'){	
				// if connected				
				if(state.status == CT){
					// Create text message				
					text.updateClientID("abc123");
					std::string msg = text.createListRequest();
					#ifdef vv					
					std::cout<<msg<<std::endl;
					#endif					
					// Create binary message
					icp.size = msg.size();
					icp.startbit = 0x00;
					icp.ackbit = 0x00;
					icp.cackbit = 0x00;
					state.seq++;			
					icp.seq = state.seq;
					icp.toBinary();
					// copy to buffer
					memcpy(buff,icp.buffer,8);
					memcpy(buff+8,msg.c_str(),icp.size);
					// send packet
					sendto(sfd,(char*)buff,8+icp.size,0,&d_addr,sizeof(d_addr));		
				}
			}
			// shows list of sensors available in sensors
			else if(option == 'l'){
				showList(list);
			}
			// send SUBSCRIBE request
			else if(option ==  's'){
				if(state.status == CT){
					// Create text message				
					text.updateClientID("abc123");
					text.updateCount(list.size());
					text.updateDeviceIDs(list);
					std::string msg = text.createSubsRequest();
					#ifdef vv					
					std::cout<<msg<<std::endl;
					#endif					
					// Create binary message
					icp.size = msg.size();
					icp.startbit = 0x00;
					icp.ackbit = 0x00;
					icp.cackbit = 0x00;
					state.seq++;			
					icp.seq = state.seq;
					icp.toBinary();
					// copy to buffer
					memcpy(buff,icp.buffer,8);
					memcpy(buff+8,msg.c_str(),icp.size);
					// send packet
					sendto(sfd,(char*)buff,8+icp.size,0,&d_addr,sizeof(d_addr));		
				}				
			}
		}	
	}
	return 0;
}
