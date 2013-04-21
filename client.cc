//IoTPS Client
#include"client.hh"
#include"icp.hh"
#include"state.hh"
#include"comm.hh"
#include"helpers.hh"
#include"screen.hh"

std::queue<std::string> outgoing;
std::queue<std::string> incoming;

// thread to handle data exchange
void *lower(void * arg){
	struct sockaddr d_addr,s_addr; //address sructure for destination,source
	State state;
	ICP icp(state.seq);
	int sfd;
	int ret;
	size_t rsize;
	socklen_t len;
	unsigned char buff[BUFF_SIZE];
	fd_set rfds;	
	struct timespec t;
	t.tv_sec = 1;
	t.tv_nsec = 0;

	// from main thread
	struct address * adr = (struct address *)arg;

	// create socket
	memset(&d_addr,0,sizeof(struct sockaddr));
	if( (sfd = custom_socket_remote(adr->ip,adr->port,&d_addr)) == -1)// server ip,port
		pthread_exit(NULL);

	// Connect
	// Send empty binary packet with start bit set	
	icp.toBinary();
	memcpy(buff,icp.buffer,8);
	sendto(sfd,(char*)buff,8,0,&d_addr,sizeof(d_addr));
	len = sizeof(s_addr);
	while(1){
		FD_ZERO(&rfds);
		FD_SET(sfd,&rfds);
		if( (ret = pselect(sfd+1,&rfds,NULL,NULL,&t,NULL)) == -1){
			continue;
		}
		if(FD_ISSET(sfd,&rfds)){
			//TODO:Check source address
			if( (rsize = recvfrom(sfd,(char*)buff,BUFF_SIZE,0,&s_addr,&len)) == -1){
				continue;
			}
			else if(rsize >= 8){
				memcpy(icp.buffer,buff,8);
				icp.toValues();
				if(icp.startbit == 0x01 && state.status == RC){			
					// Update state 
					state.status = CT;
					state.ack = icp.seq;
					// Update
					state.seq++;
					icp.startbit = 0x00;
					icp.endbit = 0x00;
					icp.ackbit = 0x01;
					icp.cackbit = 0x01; // logic here
					icp.seq = state.seq;
					icp.ack = state.ack;			
					icp.toBinary();
					memcpy(buff,icp.buffer,8);
					// send packet
					sendto(sfd,(char*)buff,8,0,&d_addr,sizeof(d_addr));
				}
				else if(icp.endbit == 0x01 && state.status == CT){
				}
				else if(icp.size != 0){	
					// Update state
					state.ack = icp.seq;				
					memmove(buff,buff+8,icp.size);
					memset(buff+icp.size,0,1000-icp.size);	
					std::string str((char*)buff);		
					incoming.push(str);
				}
			}
		}
		// data from application layer
		while(outgoing.size()){
			std::string str = outgoing.front();
			outgoing.pop();
			icp.size = str.size();
			icp.startbit = 0x00;
			icp.endbit == 0x00;
			icp.ackbit = 0x01; //read from state
			icp.cackbit = 0x01; //read from state
			icp.seq = state.seq;
			icp.ack = state.ack;
			icp.toBinary();
			// copy to buffer
			memcpy(buff,icp.buffer,8);
			memcpy(buff+8,str.c_str(),icp.size);
			// send packet
			sendto(sfd,(char*)buff,8+icp.size,0,&d_addr,sizeof(d_addr));
		}

	}
	pthread_exit(NULL);
}

int main(int argc,char *argv[]){
	CommMessage text;	
	// ncurses
	Screen scr;
	std::string cmd;
	std::vector<std::string> list;
	std::vector<std::string> slist;
	std::vector<std::string> ulist;	
	struct address adr;
	enum REQ req;
	pthread_t thread;
	int opt;
	int ch;
	int ret;
	ch = 0;
	req = NONE;

	// Parse command line options
	while( (opt = getopt(argc,argv, "s:p:")) != -1){
		switch(opt){
			case 's':
				strncpy(adr.ip,optarg,IPLEN);
				break;
			case 'p':
				strncpy(adr.port,optarg,PORTLEN);
				break;
			case '?':
				return -1;
			default:
				return -1;		
		}
	}

	// starting thread to handle data exchange
	if( (ret = pthread_create(&thread, NULL, lower, (void *)&adr)) != 0)
	{		
		return -1;
	}	

	while(1){
		wtimeout(scr.getIPWin(),1000);
		ch = wgetch(scr.getIPWin());
		if(ch == KEY_DOWN){
			scr.moveWinDown();
		}
		else if(ch == KEY_UP){
			scr.moveWinUp();
		}
		else if(ch == KEY_LEFT || ch == KEY_RIGHT){
			scr.toggle();		
		}
		else if(ch == 9){ // tab
			scr.switchtab();		
		}
		else if(ch == 27){
			return 0;		
		}
		else if(ch == 'R' || ch == 'r'){ //refresh,get sensors list	
			text.updateClientID("abc123");
			std::string msg = text.createListRequest();			
			outgoing.push(msg);
			scr.status("List request sent.");
			req = LIST;
		}
		else if(ch == 'S' || ch == 's'){
			text.updateClientID("abc123");
			slist = scr.getSList();
			text.updateDeviceIDs(slist);
			text.updateCount(slist.size());
			std::string msg = text.createSubsRequest();
			outgoing.push(msg);
			scr.status("Subs request sent.");
			req = SUBS;
		}
		else if(ch == 'U' || ch == 'u'){
			text.updateClientID("abc123");
			ulist = scr.getUList();
			text.updateDeviceIDs(ulist);
			text.updateCount(ulist.size());
			std::string msg = text.createUnsubsRequest();
			outgoing.push(msg);
			scr.status("Unsubs request sent.");
			req = UNSUBS;
		}
		// data from lower layer
		while(incoming.size()){
			text.updateMessage(incoming.front());
			incoming.pop();
			text.parse();
			cmd = text.getCommand();
			if(cmd == "OK" && req == LIST){
				list = text.getDeviceIDs();
				scr.addAList(list);
				scr.status("Press 'R' to retrive list, 'S' to subscribe and 'U' to unsubscribe.");
				req = NONE;
			}
			else if(cmd == "OK" && req == SUBS){
				//TODO
				scr.status("Press 'R' to retrive list, 'S' to subscribe and 'U' to unsubscribe.");
				req = NONE;			
			}
			else if(cmd == "OK" && req == UNSUBS){
				//TODO
				scr.status("Press 'R' to retrive list, 'S' to subscribe and 'U' to unsubscribe.");
				req = NONE;			
			} 
			else if(cmd == "UPDATE"){
				//TODO
				scr.status("Updates.");
				req = NONE;			
			}
		}
		ch = 0;					
	}
	return 0;
}
