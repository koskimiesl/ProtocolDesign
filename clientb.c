/* Client Binary protocol */
#include"clientb.h"
#include<sys/epoll.h>

int main(int argc,char *argv[]){
	unsigned char obuff[BUFF_SIZE],ibuff[BUFF_SIZE];
	unsigned char fbuff[BUFF_SIZE];
	int epollfd,nfds,n;
	struct epoll_event ev,events[2];
	struct ICP icp;
	struct sockaddr_un local;
	struct sockaddr d_addr,s_addr;
	struct State state;	
	struct list_head * pos,* q;
	struct Queue * tmp;
	struct Queue * queue;	
	struct timeval ct;
	socklen_t len;
	size_t rsize;
	len = sizeof(struct sockaddr);	
	int ufd,sfd,nfd;
	int length;
	unsigned short s;
	bool cnt;	
	size_t idx;
	memset(&local,0,sizeof(struct sockaddr_un));

	if( (ufd = socket(AF_UNIX,SOCK_STREAM,0)) == -1){
		perror("socket");
		return -1;	// NC
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH);
	unlink(local.sun_path);
	length = strlen(local.sun_path) + sizeof(local.sun_family);
	
	if( (bind(ufd,(struct sockaddr *)&local,length)) == -1){
		perror("bind");
		return -1; // NC	
	}

	if( (listen(ufd,5)) == -1){
		perror("listen");
		return -1; // NC
	}

	if( (epollfd = epoll_create(2)) == -1){
		perror("epoll_create");
		return -1; // NC
	}

	// add ufd to epoll
	ev.events = EPOLLIN;
	ev.data.fd = ufd;
	if(epoll_ctl(epollfd,EPOLL_CTL_ADD,ufd,&ev) == -1){
		perror("epoll_ctl");
		return -1; // NC	
	}

	memset(&icp,0,sizeof(struct ICP));
	
	memset(&d_addr,0,sizeof(struct sockaddr));	
	initState(&state);
	while(1){
		if( (nfds = epoll_wait(epollfd,events,2,200)) == -1){
			perror("epoll_wait");
			return -1;		
		}
		else if(nfds == 0){ //time
			if(state.ackreq){
				icp.version = 0x01;
				icp.startbit = 0x00;
				icp.endbit = 0x00;
				icp.ackbit = 0x01;
				icp.cackbit = 0x01;
				icp.kalive = 0x00;
				icp.frag = 0x00;
				icp.size = 0x0000;
				icp.seq = state.seq;
				icp.ack = state.rack;
				toBinary(&icp);
				memcpy(obuff,icp.buffer,8);
				sendto(sfd,(char*)obuff,8,0,&d_addr,sizeof(d_addr));	
				state.ackreq = false;							
			}
		//	gettimeofday(&ct,NULL);
		//	list_for_each(pos,&(state.out.list)){
		//		tmp = list_entry(pos,struct Queue,list);
		//		if( ((ct.tv_sec - tmp->st.tv_sec) > 1 )	|| ( ((ct.tv_sec - tmp->st.tv_sec) >= 0 ) && ((ct.tv_usec - tmp->st.tv_sec) > 500000) ) ){
		//			sendto(sfd,(char *)tmp->buffer,tmp->size,0,&d_addr,sizeof(d_addr));				
		//		} 				
		//	}		
			continue;
		}
	
		for(n = 0; n < nfds; n++){
			
			if(events[n].data.fd == ufd){
				// create udp socket to remote host
				if( (sfd = custom_socket_remote(argv[0],argv[1],&d_addr)) == -1){ // server ip,port
					return -1;					
				}
				// Create packet with start bit set	
				icp.version = 0x01;
				icp.startbit = 0x01;
				icp.endbit = 0x00;
				icp.ackbit = 0x00;
				icp.cackbit = 0x00;
				icp.kalive = 0x00;
				icp.frag = 0x00;
				icp.size = 0x0000;
				icp.seq = state.seq;
				icp.ack = 0x0000;
				toBinary(&icp);
				memcpy(obuff,icp.buffer,8);
				// add to state memory
				addOutPacketToState(&state,obuff,icp.seq,8);
				// send connect packet
				sendto(sfd,(char*)obuff,8,0,&d_addr,sizeof(d_addr));
				memcpy(&(state.addr),&d_addr,len);
				state.len = sizeof(struct sockaddr);		
				// reomove ufd
				ev.events = EPOLLIN;
				ev.data.fd = ufd;
				if(epoll_ctl(epollfd,EPOLL_CTL_DEL,ufd,&ev) == -1){
					perror("epoll_ctl,ufd");
					return -1;	
				}
				// add sfd
				ev.events = EPOLLIN;
				ev.data.fd = sfd;
				if(epoll_ctl(epollfd,EPOLL_CTL_ADD,sfd,&ev) == -1){
					perror("epoll_create,sfd");
					return -1;	
				}	
			}
			else if(events[n].data.fd == sfd){
				//TODO:Check source address
				if( (rsize = recvfrom(sfd,(char*)ibuff,BUFF_SIZE,0,&s_addr,&len)) == -1){
					continue;				
				}			
				else if(rsize >= 8){
					memcpy(icp.buffer,ibuff,8);
					toValues(&icp);
					if(icp.startbit == 0x01 && state.status == RC){
						// Update state 
						state.status = CT;	
						ackThis(&state,icp.ack,icp.ackbit,icp.cackbit);	
						state.rack = icp.seq;
						state.sentup = icp.seq;			
						// Update
						icp.version = 0x01; 
						icp.startbit = 0x00;
						icp.endbit = 0x00;
						icp.ackbit = 0x01;
						icp.cackbit = 0x01;
						icp.kalive = 0x00;
						icp.frag = 0x00;
						icp.size = 0x0000;
						icp.seq = state.seq;
						icp.ack = state.rack;			
						toBinary(&icp);
						memcpy(obuff,icp.buffer,8);
						// send packet
						sendto(sfd,(char*)obuff,8,0,&d_addr,sizeof(d_addr));
						nfd = accept(ufd,NULL,NULL);
						// add sfd
						ev.events = EPOLLIN;
						ev.data.fd = nfd;
						if(epoll_ctl(epollfd,EPOLL_CTL_ADD,nfd,&ev) == -1){
							perror("epoll_create,nfd");
							return -1;	
						}
					}
					else if(icp.endbit == 0x01 && state.status == CT){
					}
					else if(icp.ackbit == 0x01 && icp.size == 0){
						// Ack 
						ackThis(&state,icp.ack,icp.ackbit,icp.cackbit);
					}
					else if(icp.kalive = 0x01 && icp.size == 0){
					}
					else if(icp.size != 0 && state.status == CT){
						// Ack
						ackThis(&state,icp.ack,icp.ackbit,icp.cackbit);
						if(!ackThat(&state,icp.seq)){
							// Immediate ack
							icp.version = 0x01; 
							icp.startbit = 0x00;
							icp.endbit = 0x00;
							icp.ackbit = 0x01;
							icp.cackbit = 0x00;
							icp.kalive = 0x00;
							icp.frag = 0x00;
							icp.size = 0x0000;
							icp.ack = icp.seq;
							icp.seq = state.seq;
							toBinary(&icp);
							memcpy(obuff,icp.buffer,8);
							sendto(sfd,(char*)obuff,8,0,&d_addr,sizeof(d_addr));
						}			
						// store			
						memmove(obuff,ibuff+8,icp.size);
						addInPacketToState(&state,obuff,icp.seq,icp.size,icp.frag);						
						// Send up
						while(1){
							cnt = false;
							s = ((state.sentup)==65535)?1:(state.sentup)+1;
							list_for_each_safe(pos,q,&(state.in.list)){
								queue = list_entry(pos,struct Queue,list);
								if(queue->seq == s){
									state.sentup = s;
									memcpy(fbuff+idx,queue->buffer,queue->size);
									idx += queue->size;
									if(!(queue->frag)){
										send(nfd,fbuff,idx,0);
										idx = 0;									
									}
									list_del(pos);
									free(queue);
									cnt = true;
									break;				
								}
							}				
							if(!cnt)
								break;				
						}
					}	
				}
			}
            else if(events[n].data.fd == nfd){
				if( (rsize = recv(nfd,(char *)ibuff,BUFF_SIZE,0)) > 0){
					// Increase state.seq
					state.seq=(state.seq==65535)?1:state.seq+1;
					// Update
					icp.version = 0x01;
					icp.startbit = 0x00;
					icp.endbit = 0x00;
					if(state.ackreq){
						icp.ackbit = 0x01;
						icp.cackbit = 0x01;
						state.ackreq = false;
					}
					else {
						icp.ackbit = 0x00;
						icp.cackbit = 0x00;
					}
					icp.kalive = 0x00;
					icp.size = rsize;
					icp.seq = state.seq; 
					icp.ack = state.rack; // invalid if ackbit is 0
					toBinary(&icp);
					memcpy(obuff,icp.buffer,8);
					memcpy(obuff+8,ibuff,rsize);
					sendto(sfd,(char*)obuff,8+rsize,0,&(state.addr),state.len);							
					// add to state memory
					addOutPacketToState(&state,obuff,icp.seq,8+rsize);				
				}			
				else if(rsize == 0){
					// sent endbit
				}
			}
		}
	}
	return 0;
}
