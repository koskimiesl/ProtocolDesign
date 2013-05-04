/* Client Binary protocol */
#include"clientb.hh"

//some timer mechanism

int main(int argc,char *argv[]){
	unsigned char buff[BUFF_SIZE];
	unsigned char obuff[BUFF_SIZE];
	unsigned char ibuff[BUFF_SIZE];
	int nfd,ufd,sfd;
	int ret;
	int maxfd;
	int length;
	State state;
	ICP icp(state.seq);
	struct sockaddr_un local;
	struct sockaddr d_addr,s_addr;
	socklen_t len;
	size_t rsize;
	fd_set rfds;
	struct timespec t;
	t.tv_sec = 1;
	t.tv_nsec = 0;
	len = sizeof(struct sockaddr);	
	
	memset(&d_addr,0,sizeof(struct sockaddr));
	if( (sfd = custom_socket_remote(argv[0],argv[1],&d_addr)) == -1)// server ip,port
		return -1;
	maxfd = sfd;
	memset(&local,0,sizeof(struct sockaddr_un));
	if( (ufd = socket(AF_UNIX,SOCK_STREAM,0)) == -1)
		perror("socket");

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH);
	unlink(local.sun_path);
	length = strlen(local.sun_path) + sizeof(local.sun_family);
	
	if( (bind(ufd,(struct sockaddr *)&local,length)) == -1)
		perror("bind");
	if( (listen(ufd,5)) == -1)
		return -1;
	nfd = accept(ufd,NULL,NULL);
	// Connect
	// Send empty binary packet with start bit set	
	icp.toBinary();
	memcpy(buff,icp.buffer,8);
	sendto(sfd,(char*)buff,8,0,&d_addr,sizeof(d_addr));
	maxfd = (nfd > maxfd)?nfd:maxfd;
	while(1){
		FD_ZERO(&rfds);
		FD_SET(nfd,&rfds);
		FD_SET(sfd,&rfds);
		if( (ret = pselect(maxfd+1,&rfds,NULL,NULL,&t,NULL)) == -1){
			continue;
		}
		else if(FD_ISSET(nfd,&rfds)){
			rsize = recv(nfd,(char *)ibuff,BUFF_SIZE,0);
			state.seq++; //wrap
			icp.size = rsize;
			icp.startbit = 0x00;
			icp.endbit == 0x00;
			icp.ackbit = 0x01;  // read from state
			icp.cackbit = 0x01; // read from state
			icp.seq = state.seq;
			icp.ack = state.ack;
			icp.toBinary();
			// copy to buffer
			memcpy(obuff,icp.buffer,8);
			memcpy(obuff+8,ibuff,rsize);
			// send packet
			sendto(sfd,(char*)obuff,8+icp.size,0,&d_addr,sizeof(d_addr));
		}
		else if(FD_ISSET(sfd,&rfds)){
			// TODO:Check source address
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
					state.seq++; //wrap
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
					// for last
				}
				else if(icp.size != 0){	
					// Update state
					state.ack = icp.seq;				
					memmove(buff,buff+8,icp.size);
					send(nfd,buff,icp.size,0);
				}
			}
		}
	}	
	return 0;
}

