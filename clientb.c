/* Client Binary protocol */
#include"clientb.h"

/* Handle signal */
void signalhandler(int signo){
	switch(signo){
	}
}

/* Write packets to udp socket. */
void sendPacket(struct State * state,int sfd){
	unsigned char obuff[1008];
	unsigned short x;
	bool cnt;
	struct ICP icp;
	struct list_head *pos;
	struct Queue *queue;	
	memset(&icp,0,sizeof(struct ICP));
	while( (state->window) > 1000){
		cnt = false;
		x = ((state->sentdown)==65535)?1:(state->sentdown)+1;
		list_for_each(pos,&(state->out.list)){
			queue = list_entry(pos,struct Queue,list);
			if(queue->seq == x){
				gettimeofday(&(queue->st),NULL);
				queue->sent = true;
				/* Update ICP */
				updateICP(&icp,0x00,0x00,0x01,0x01,0x00,queue->frag,
									queue->size,queue->seq,state->rack);
				state->ackreq = false;
				toBinary(&icp);
				printICPOut(&icp);
				memcpy(obuff,icp.buffer,8);
				memcpy(obuff+8,queue->buffer,queue->size);
				sendto(sfd,(char *)obuff,(queue->size)+8,0,&(state->addr),state->len);
				(state->window) -= (queue->size);
				(state->sentdown) = x;
				cnt = true;
				continue;
			}
		}
		if(!cnt)
			break;
	}
}

void sendAck(struct State * state,int sfd){
	struct ICP icp;
	char obuff[1008];	
	memset(&icp,0,sizeof(struct ICP));
	updateICP(&icp,0x00,0x00,0x01,0x01,0x00,0x00,0x0000,state->seq,state->rack);
	toBinary(&icp);
	printICPOut(&icp);
	memcpy(obuff,icp.buffer,8);
	sendto(sfd,(char*)obuff,8,0,&(state->addr),state->len);
	state->ackreq = false;
}

void sendIAck(struct State * state,unsigned short ack,int sfd){
	struct ICP icp;
	char obuff[1008];	
	memset(&icp,0,sizeof(struct ICP));
	updateICP(&icp,0x00,0x00,0x01,0x00,0x00,0x00,0x0000,state->seq,ack);
	toBinary(&icp);
	printICPOut(&icp);
	memcpy(obuff,icp.buffer,8);
	sendto(sfd,(char*)obuff,8,0,&(state->addr),state->len);
}

void timeout(struct State * state,int sfd){
	char obuff[1008];
	struct ICP icp;
	struct list_head *pos;
	struct timeval ct;
	struct Queue * queue;	
	memset(&icp,0,sizeof(struct ICP));
	sendPacket(state,sfd);	
	if(state->ackreq)
		sendAck(state,sfd);
	gettimeofday(&ct,NULL);
	//if no acks arrive	
	state->window += 2;
	list_for_each(pos,&(state->out.list)){
		queue = list_entry(pos,struct Queue,list);
		if(queue->sent && checktime(&(queue->st),&ct,0) && state->window > 2*(queue->size))
			state->window = (state->window)-(2*queue->size); 			
			/* Update ICP */
			updateICP(&icp,0x00,0x00,0x01,0x01,0x00,queue->frag,
								queue->size,queue->seq,state->rack);
			state->ackreq = false;
			toBinary(&icp);
			printICPOut(&icp);
			memcpy(obuff,icp.buffer,8);
			memcpy(obuff+8,queue->buffer,queue->size);
			sendto(sfd,(char *)obuff,(queue->size)+8,0,&(state->addr),state->len);
			gettimeofday(&(queue->st),NULL);	
	}
}


/* Create a unix socket and listen */
create_listen(char * n){
	int ufd,length;
	struct sockaddr_un local;

	if( (ufd = socket(AF_UNIX,SOCK_STREAM,0)) == -1){
		perror("socket, ");
		return -1;	
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path,n);
	unlink(local.sun_path);
	length = strlen(local.sun_path) + sizeof(local.sun_family);
	
	if( (bind(ufd,(struct sockaddr *)&local,length)) == -1){
		perror("bind, ");
		return -1;
	}

	if( (listen(ufd,5)) == -1){
		perror("listen, ");
		close(ufd);
		return -1;
	}
	return ufd;
}

int main(int argc,char *argv[]){
	socklen_t len;
	size_t k,t,tsize,idx;
	int rsize,ret;	
	int ufd,sfd,nfd;
	int epollfd,nfds,n;	
	bool confirm,cnt;
	unsigned char obuff[1008],ibuff[BUFF_SIZE],fbuff[20000];
	unsigned char tfrag;
	unsigned short s;
	struct State state;
	struct epoll_event ev,events[2];
	struct ICP icp;
	struct sockaddr d_addr,s_addr;	
	struct timeval ct,pt;
	struct list_head *pos,*q;
	struct Queue * queue;
	enum Event event;
	int fd;

	if( (ufd = create_listen(argv[4])) == -1){
		raise(SIGUSR1);		
	}	
	
	if( (epollfd = epoll_create(2)) == -1){
		perror("epoll_create");
		raise(SIGUSR1);
	}

	/* Add ufd to epoll */
	ev.events = EPOLLIN;
	ev.data.fd = ufd;
	if(epoll_ctl(epollfd,EPOLL_CTL_ADD,ufd,&ev) == -1){
		perror("epoll_ctl, ");
		raise(SIGUSR1);
	}
	
	len = sizeof(struct sockaddr);
	memset(&icp,0,sizeof(struct ICP));
	initState(&state);	
	sfd = -1;
	confirm = false;
	fd = open("logclient.txt",O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	dup2(fd,1);
	int count;
	int code;
	count = 0;
	while(1){
		if( (nfds = epoll_wait(epollfd,events,2,600)) == -1){
			perror("epoll_wait, ");
			raise(SIGUSR1);		
		}
		else if(nfds == 0){
			if(sfd != -1 && state.status == RC){
				/* Create packet with start bit set */
				updateICP(&icp,0x01,0x00,0x00,0x00,0x00,0x00,0x0000,state.seq,0x0000);
				toBinary(&icp);
				printICPOut(&icp);				
				memcpy(obuff,icp.buffer,8);
				/* Send connect packet */
				sendto(sfd,(char*)obuff,8,0,&d_addr,sizeof(d_addr));	
			}
			if(confirm)
				break;
			continue;
		}
		for(n = 0; n < nfds; n++){
			if(events[n].data.fd == ufd){
				/* Create udp socket to remote host */
				if( (sfd = custom_socket_remote(argv[0],argv[1],argv[5],&d_addr)) == -1){
					//TODO				
				}
				/* Create packet with start bit set */
				state.sentdown = state.seq;
				updateICP(&icp,0x01,0x00,0x00,0x00,0x00,0x00,0x0000,state.seq,0x0000);	
				toBinary(&icp);
				printICPOut(&icp);
				memcpy(obuff,icp.buffer,8);
				/* Send connect packet */
				sendto(sfd,(char*)obuff,8,0,&d_addr,sizeof(d_addr));
				memcpy(&(state.addr),&d_addr,sizeof(struct sockaddr));
				state.len = sizeof(struct sockaddr);		
				/* reomove ufd */
				ev.events = EPOLLIN;
				ev.data.fd = ufd;
				if(epoll_ctl(epollfd,EPOLL_CTL_DEL,ufd,&ev) == -1){
					perror("epoll_ctl, ");	
				}
				/* Add sfd */
				ev.events = EPOLLIN;
				ev.data.fd = sfd;
				if(epoll_ctl(epollfd,EPOLL_CTL_ADD,sfd,&ev) == -1){
					perror("epoll_ctl, ");
					raise(SIGUSR1);
				}	
			}
			else if(events[n].data.fd == sfd){
				/* TODO:Check source address */
				if( (rsize = recvfrom(sfd,(char*)ibuff,BUFF_SIZE,0,&s_addr,&len)) == -1){
					continue;				
				}
				else if(rsize == 8){
					memcpy(icp.buffer,ibuff,8);
					if(icp.version != 0x01)
						continue;					
					toValues(&icp);
					printICPIn(&icp);
					if(icp.startbit == 0x01 && state.status == RC){
						/* Update state */ 
						state.status = CT;		
						state.rack = icp.seq;
						state.sentup = icp.seq;
						state.ack = (state.ack == 65535)?1:(state.ack)+1;			
						updateICP(&icp,0x00,0x00,0x01,0x01,0x00,0x00,0x0000,state.seq,state.rack);
						toBinary(&icp);
						printICPOut(&icp);						
						memcpy(obuff,icp.buffer,8);
						confirm = true;					
						/* Send packet */
						sendto(sfd,(char*)obuff,8,0,&d_addr,sizeof(d_addr));
					}
					else if(icp.startbit == 0x01 && state.status == CT && state.rack == icp.seq){
						updateICP(&icp,0x00,0x00,0x01,0x01,0x00,0x00,0x0000,state.seq,state.rack);
						toBinary(&icp);
						printICPOut(&icp);
						memcpy(obuff,icp.buffer,8);
						confirm = true;					
						/* Send packet */
						sendto(sfd,(char*)obuff,8,0,&d_addr,sizeof(d_addr));
					}
					printState(&state);
				}			
			}
		}
	}

	nfd = accept(ufd,NULL,NULL);
	/* Add nfd */
	ev.events = EPOLLIN;
	ev.data.fd = nfd;
	if(epoll_ctl(epollfd,EPOLL_CTL_ADD,nfd,&ev) == -1){
		perror("epoll_ctl, ");
		raise(SIGUSR1);	
	}
	setngetS(nfd);
	
	idx = 0;
	gettimeofday(&pt,NULL);
	gettimeofday(&(state.kt),NULL);
	event = RECEIVED;
	while(1){
		if( (nfds = epoll_wait(epollfd,events,2,200)) == -1){
			perror("epoll_wait, ");
			raise(SIGUSR1);		
		}
		else if(nfds == 0){
			timeout(&state,sfd);
			gettimeofday(&pt,NULL);				
		}
		for(n = 0; n < nfds; n++){
			if(events[n].data.fd == sfd){
				/* TODO:Check source address */
				if( (rsize = recvfrom(sfd,(char*)ibuff,BUFF_SIZE,0,&s_addr,&len)) == -1){
					continue;				
				}
				else if(rsize >= 8){
					event = getNextState(event,strtod(argv[2],NULL),strtod(argv[3],NULL));
					if(!event)
						continue;
					gettimeofday(&(state.kt),NULL);
					memcpy(icp.buffer,ibuff,8);
					toValues(&icp);
					printICPIn(&icp);
					if( icp.version != 0x01)
						continue;
					if(icp.endbit == 0x01 && state.status == CT){
						/* Remove nfd */
						ev.events = EPOLLIN;
						ev.data.fd = nfd;
						if(epoll_ctl(epollfd,EPOLL_CTL_DEL,nfd,&ev) == -1){
							perror("epoll_ctl, ");
							raise(SIGUSR1);	
						}					
						close(nfd);
						close(ufd);	
						/* Send ack */
						updateICP(&icp,0x00,0x01,0x00,0x00,0x00,0x00,0x0000,state.seq,icp.seq);
						toBinary(&icp);
						printICPOut(&icp);
						memcpy(obuff,icp.buffer,8);
						sendto(sfd,(char*)obuff,8,0,&(state.addr),state.len);
						break;
					}
					else if(icp.ackbit == 0x01 && icp.size == 0){
						/* Ack */ 
						ackThis(&state,icp.ack,icp.ackbit,icp.cackbit);
					}
					else if(icp.kalive = 0x01 && icp.size == 0){
						/* Send Keepalive packet */
						updateICP(&icp,0x00,0x00,0x00,0x00,0x01,0x00,0x0000,state.seq,0x0000);
						toBinary(&icp);
						memcpy(obuff,icp.buffer,8);
						sendto(sfd,(char*)obuff,8,0,&(state.addr),state.len);
					}
					else if(icp.size != 0 && state.status == CT){
						if(state.rack > 32768){
							if(icp.seq <= state.rack && icp.seq > ((state.rack)-32768)){
								//send ack							
								sendAck(&state,sfd);		
								continue;
							}
						} 
						else if(state.rack <= 32768){
							if( icp.seq <= state.rack || icp.seq > (32767+(state.rack)) ){
								//send ack
								sendAck(&state,sfd);
								continue;
							}
						}
						/* Ack */
						ackThis(&state,icp.ack,icp.ackbit,icp.cackbit);
						ret = ackThat(&state,icp.seq);
						if(ret == 0 || ret == -1){
							/* Immediate ack */
							sendIAck(&state,icp.seq,sfd);
						}			
						if(ret == -1)
							continue;
						/* Store */			
						memmove(obuff,ibuff+8,icp.size);
						addInPacketToState(&state,obuff,icp.seq,icp.size,icp.frag);						
						/* Send up */
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
										code=send(nfd,fbuff,idx,0);									
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
					printState(&state);	
				}			
			}
			else if(events[n].data.fd == nfd){
				if( (rsize = recv(nfd,(char *)ibuff,BUFF_SIZE,0)) > 0){
					t = (rsize-1)/1000;
					for(k = 0; k <= t; k++){
						state.seq = ((state.seq)==65535)?1:(state.seq)+1;
						tsize = (rsize > 1000)?1000:rsize;
						tfrag = (k == t)?0x00:0x01;
						memcpy(obuff,ibuff+k*1000,tsize);
						addOutPacketToState(&state,obuff,state.seq,tsize,tfrag);
						rsize-=1000;
					}
					/* Send packet,if network is congestion free */ 
					sendPacket(&state,sfd);				
				}			
				else if(rsize == 0){
					/* Remove nfd */
					ev.events = EPOLLIN;
					ev.data.fd = nfd;
					if(epoll_ctl(epollfd,EPOLL_CTL_DEL,nfd,&ev) == -1){
						perror("epoll_ctl, ");
						raise(SIGUSR1);	
					}
					/* Remove sfd */
					ev.events = EPOLLIN;
					ev.data.fd = sfd;
					if(epoll_ctl(epollfd,EPOLL_CTL_DEL,sfd,&ev) == -1){
						perror("epoll_ctl, ");
						raise(SIGUSR1);	
					}					
					close(nfd);
					close(ufd);
					close(sfd);
					close(fd);
					releaseState(&state);
					return 0;
				}
			}
		}
		gettimeofday(&ct,NULL);
		if( (ct.tv_sec - state.kt.tv_sec) > 120){
			/* Remove nfd */
			ev.events = EPOLLIN;
			ev.data.fd = nfd;
			if(epoll_ctl(epollfd,EPOLL_CTL_DEL,nfd,&ev) == -1){
				perror("epoll_ctl, ");
				raise(SIGUSR1);	
			}
			/* Remove sfd */
			ev.events = EPOLLIN;
			ev.data.fd = sfd;
			if(epoll_ctl(epollfd,EPOLL_CTL_DEL,sfd,&ev) == -1){
				perror("epoll_ctl, ");
				raise(SIGUSR1);	
			}					
			close(nfd);
			close(ufd);
			close(sfd);
			close(fd);
			releaseState(&state);
			return 0;
		}

		if(checktime(&pt,&ct,200000)){
			timeout(&state,sfd);
			gettimeofday(&pt,NULL);
		}						
	}
	raise(SIGUSR1);
	return 0;
}
