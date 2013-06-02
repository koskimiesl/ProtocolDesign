/* Server Binary Protocol */
#include"serverb.h"

/* Handle signal */
void signalhandler(int signo){
	switch(signo){
	}
}

/* Create a unix socket and connect to server (for each client) */
int create_connect(char * n){
	int ufd,length;
	struct sockaddr_un remote;
	
	if( (ufd = socket(AF_UNIX,SOCK_STREAM,0)) == -1){
		perror("socket, ");
		return -1;
	}

	memset(&remote,0,sizeof(struct sockaddr_un));
	remote.sun_family = AF_UNIX;	
	strcpy(remote.sun_path,n);
	length = strlen(remote.sun_path) + sizeof(remote.sun_family);
	
	if( connect(ufd,(struct sockaddr*)&remote,length) == -1){
		perror("connect, ");
		close(ufd);
		return -1;	
	}
	setngetR(ufd);
	return ufd;
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
				sendto(sfd,(char *)obuff,queue->size+8,0,&(state->addr),state->len);
				state->window = (state->window)-(queue->size); 
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

void timeout(struct State * s,int sfd){
	struct State *state;
	struct Queue *queue;
	struct list_head *pos,*idx;	
	struct timeval ct;
	struct ICP icp;	
	char obuff[1008];	
	memset(&state,0,sizeof(struct ICP));
	
	list_for_each(pos,&(s->list)){
		state = list_entry(pos,struct State,list);
		/* Try to send remaining packets limited due to congestion window */
		sendPacket(state,sfd); 
		if(state->ackreq)
			sendAck(state,sfd);
		gettimeofday(&ct,NULL);

		if( (ct.tv_sec - state->rkt.tv_sec) > 30 ){
			/* Send Keepalive packet */
			gettimeofday(&(state->rkt),NULL);
			updateICP(&icp,0x00,0x00,0x00,0x00,0x01,0x00,0x0000,state->seq,0x0000);
			toBinary(&icp);
			printICPOut(&icp);
			memcpy(obuff,icp.buffer,8);
			sendto(sfd,(char*)obuff,8,0,&(state->addr),state->len);
		}
		// if no acks arrive
		state->window += 100;
		list_for_each(idx,&(state->out.list)){
			queue = list_entry(idx,struct Queue,list);
			if( queue->sent && checktime(&(queue->st),&ct,0) && state->window > 2*(queue->size) ){
				state->window = (state->window)-(2*queue->size); 
				/* Update ICP */
				updateICP(&icp,0x00,0x00,0x01,0x01,0x00,queue->frag,queue->size,queue->seq,state->rack);
				toBinary(&icp);
				printICPOut(&icp);
				memcpy(obuff,icp.buffer,8);
				memcpy(obuff+8,queue->buffer,queue->size);
				sendto(sfd,(char*)obuff,8+(queue->size),0,&(state->addr),state->len);
				gettimeofday(&(queue->st),NULL);
			}				
		}
	}
}

int main(int argc,char *argv[]){
	int temp,ret; 
	int epollfd,sfd;
	int nfds,n,rsize;
	socklen_t len;
	size_t idx;
	size_t k,t;	
	struct ICP icp;
	struct sockaddr addr;
	struct epoll_event ev,events[1000];	
	struct State state_list;
	struct State * state;
	struct Queue * queue;
	struct timeval ct,pt;
	struct list_head * pos,*q;
	unsigned short s,tsize;
	unsigned char tfrag;	
	unsigned char ibuff[BUFF_SIZE];	
	unsigned char obuff[BUFF_SIZE];
	unsigned char fbuff[BUFF_SIZE];	
	bool cnt;
	int Ufcount,fcount;
	Ufcount = 0;
	fcount = 0;	
	idx = 0;
	len = sizeof(struct sockaddr);
	memset(&icp,0,sizeof(struct ICP));

	/* Create udp socket. AF_INET,AF_INET6 or AF_UNSPEC */	
	if( (sfd = custom_socket(AF_INET,argv[0],argv[2])) == -1){
		raise(SIGUSR1);
	}
	
	if( (epollfd = epoll_create(2)) == -1){
		perror("epoll_create, ");
		raise(SIGUSR1);
	}

	/* Add sfd to epoll */
	ev.events = EPOLLIN;
	ev.data.fd = sfd;
	if(epoll_ctl(epollfd,EPOLL_CTL_ADD,sfd,&ev) == -1){
		perror("epoll_ctl, ");
		raise(SIGUSR1);
	}

	INIT_LIST_HEAD(&state_list.list);
	gettimeofday(&pt,NULL);
	while(1){
		if( (nfds = epoll_wait(epollfd,events,1000,200)) == -1){
			perror("epoll_wait, ");
			raise(SIGUSR1);		
		}
		else if(nfds == 0){
			timeout(&state_list,sfd);
			gettimeofday(&pt,NULL);
		}
		for(n = 0; n < nfds; n++){
			if(events[n].data.fd == sfd){
				memset((char*)ibuff,0,BUFF_SIZE);
				if( (rsize = recvfrom(sfd,(char*)ibuff,BUFF_SIZE,0,&addr,&len)) == -1){
					continue;
				}
				else if(rsize >= 8){
					memcpy(icp.buffer,ibuff,8);
					toValues(&icp);
					if(icp.version != 0x01)
						continue;
					printICPIn(&icp);
					if(	(state = findState_addr(&state_list,&addr)) == NULL){
						/* It must contain start bit */
						/* Limited check. */
						if(icp.startbit == 0x01 && icp.size == 0){
							state = (struct State *)malloc(sizeof(struct State));
							initState(state);
							memcpy(&(state->addr),&addr,len);
							state->len = len;
							state->rack = icp.seq;
							state->sentup = icp.seq;
							gettimeofday(&(state->kt),NULL);
							gettimeofday(&(state->rkt),NULL);
							/* Send packet with start bit set */
							updateICP(&icp,0x01,0x00,0x01,0x01,0x00,0x00,
											0x0000,state->seq,state->rack);
							state->sentdown = state->seq;
							toBinary(&icp);
							printICPOut(&icp);
							memcpy(obuff,icp.buffer,8);
							printState(state);							
							/* Add to state memory */
							addOutPacketToState(state,obuff,icp.seq,0,0x00);
							list_add_tail(&(state->list),
											&(state_list.list));
							sendto(sfd,(char*)obuff,8,0,
											&(state->addr),state->len);								
						} 
					}
					else {
						gettimeofday(&(state->kt),NULL);
						gettimeofday(&(state->rkt),NULL);
						if(icp.endbit == 0x01 && icp.size == 00){
						}
						else if(icp.ackbit == 0x01 && icp.size == 0){
							if(state->status == RC){
								if( (temp = create_connect(argv[1])) == -1)
									continue; // Discuss,delete ??
								state->fd = temp;
								state->status = CT;
								/* Add temp to epoll */
								ev.events = EPOLLIN;			
								ev.data.fd = temp;
								if(epoll_ctl(epollfd,EPOLL_CTL_ADD,temp,&ev) == -1){
									perror("epoll_ctl, ");
								}
							}
							/* Ack */
							ackThis(state,icp.ack,icp.ackbit,icp.cackbit);				
						}
						else if(icp.kalive == 0x01 && icp.size == 0){
							// good						
						}
						else if(icp.size != 0){
							if(state->rack > 32768){
								if(icp.seq <= state->rack && icp.seq > ((state->rack)-32768)){
									//send ack							
									sendAck(state,sfd);		
									continue;
								}
							} 
							else if(state->rack <= 32768){
								if( icp.seq <= state->rack || icp.seq > (32767+(state->rack)) ){
									//send ack
									sendAck(state,sfd);
									continue;
								}
							}							
							/* Ack */
							ackThis(state,icp.ack,icp.cackbit,icp.cackbit);
							ret = ackThat(state,icp.seq);	
							if(ret == 0 || ret == -1){
								/* Immediate ack */
								sendIAck(state,icp.seq,sfd);						
							}	
							if(ret == -1)
								continue;
							memmove(obuff,ibuff+8,icp.size);
							addInPacketToState(state,obuff,icp.seq,icp.size,icp.frag);
							/* Send up */
							while(1){
								cnt = false;
								s = ((state->sentup)==65535)?1:(state->sentup)+1;
								list_for_each_safe(pos,q,&(state->in.list)){
									queue = list_entry(pos,struct Queue,list);
									if(queue->seq == s){
										state->sentup = s;
										memcpy(fbuff+idx,queue->buffer,queue->size);
										idx += queue->size;
										if(!(queue->frag)){
											send(state->fd,fbuff,idx,0);
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
						printState(state);					
					}				
				}
			}
			else {
				if( (state = findState_fd(&state_list,events[n].data.fd)) != NULL){
					if( (rsize = recv(events[n].data.fd,(char *)ibuff,BUFF_SIZE,0)) > 0){
						t = (rsize-1)/1000;
						for(k = 0; k <= t; k++){
							state->seq = ((state->seq)==65535)?1:(state->seq)+1;
							tsize = (rsize > 1000)?1000:rsize;
							tfrag = (k == t)?0x00:0x01;
							memcpy(obuff,ibuff+k*1000,tsize);
							addOutPacketToState(state,obuff,state->seq,tsize,tfrag);
							rsize-=1000;
						}
						/* Send packet,if network is congestion free */ 
						sendPacket(state,sfd);
					}				
					else if(rsize == 0){
						state->clean = true;
					}
					else
						perror("recv, ");
				}				
			}	
		}
		gettimeofday(&ct,NULL);
		list_for_each_safe(pos,q,&(state_list.list)){
			state=list_entry(pos,struct State,list);
			if( (ct.tv_sec - state->kt.tv_sec) > 120 || state->clean ){
				list_del(pos);
				releaseState(state);	
				/* Remove fd from epoll */
				ev.events = EPOLLIN;
				ev.data.fd = state->fd;
				if(epoll_ctl(epollfd,EPOLL_CTL_DEL,state->fd,&ev) == -1){
					perror("epoll_ctl, ");
					raise(SIGUSR1);
				}			
				close(state->fd);
				free(state);		
			}
		}
		
		if(checktime(&pt,&ct,200000)){
			timeout(&state_list,sfd);
			gettimeofday(&pt,NULL);
		}
	}
	raise(SIGUSR1);
}
