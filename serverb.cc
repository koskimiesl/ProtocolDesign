/*Server Binary Protocol */

#include"serverb.hh"

int create_connect(){
	int ufd,length;
	struct sockaddr_un remote;
	ufd = socket(AF_UNIX,SOCK_STREAM,0);
	memset(&remote,0,sizeof(struct sockaddr_un));
	remote.sun_family = AF_UNIX;	
	strcpy(remote.sun_path, SOCK_PATH);
	
	length = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if( connect(ufd,(struct sockaddr*)&remote,length) == -1);
		perror("connect");
	return ufd;
}

int main(int argc,char *argv[]){
	int ret;  // return status
	int temp; // use for misc. purpose
	int sfd;
	int maxfd;
	socklen_t len;
	size_t rsize;
	fd_set rfds;
	ICP icp;
	struct sockaddr addr;
	struct timespec t;	
	unsigned char ibuff[BUFF_SIZE];	
	unsigned char obuff[BUFF_SIZE];	
	std::vector<int> fds;
	std::vector<State> states;
	std::vector<State>::iterator itr;
	// Initialize	
	t.tv_sec = 1;
	t.tv_nsec = 0;
	len = sizeof(addr);

	if( (sfd = custom_socket(AF_INET,argv[0])) == -1) //AF_INET,AF_INET6 or AF_UNSPEC
		return -1;
	fds.push_back(sfd);
	while(1){
		FD_ZERO(&rfds);
		for(size_t c = 0;c < fds.size();c++)
			FD_SET(fds[c],&rfds);
		std::sort(fds.begin(),fds.end());
		maxfd = fds[fds.size()-1] + 1; //check
		if( (ret = pselect(maxfd,&rfds,NULL,NULL,&t,NULL)) == -1){
			continue;
		}
		for(size_t c = 0;c < fds.size();c++){
			if(FD_ISSET(fds[c],&rfds)){
				if(fds[c] == sfd){
					if( (rsize = recvfrom(sfd,(char*)ibuff,BUFF_SIZE,0,&addr,&len)) == -1){
						continue;
					}
					else if(rsize >= 8){
						memcpy(icp.buffer,ibuff,8);
						icp.toValues();
						if(icp.startbit == 0x01){								
							State state;
							state.status = CT;
							state.ack = icp.seq;
							memcpy(&state.addr,&addr,len);
							state.len = len;
							temp = create_connect();
							std::cout<<temp<<std::endl;					
							state.fd = temp;
							fds.push_back(temp);
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
							memcpy(obuff,icp.buffer,8);
							// send packet
							sendto(sfd,(char*)obuff,8,0,&addr,len);
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
							memcpy(obuff,ibuff+8,icp.size);	
							send((*itr).fd,obuff,icp.size,0);
						}	
					}				
				}
				else {
					rsize = recv(fds[c],(char *)ibuff,BUFF_SIZE,0);
					for (itr = states.begin();itr != states.end();itr++){
						if(itr->isEqual(fds[c])){				
							break;	
						}		
					}
					icp.size = rsize;
					icp.startbit = 0x00;
					icp.endbit == 0x00;
					icp.ackbit = 0x01; //read from state
					icp.cackbit = 0x01; //read from state
					icp.seq = (*itr).seq;
					icp.ack = (*itr).ack;
					icp.toBinary();
					// copy to buffer
					memcpy(obuff,icp.buffer,8);
					memcpy(obuff+8,ibuff,icp.size);
					// send packet
					sendto(sfd,(char*)obuff,8+icp.size,0,&(*itr).addr,(*itr).len);
				}
			}	
		}
	}
}
