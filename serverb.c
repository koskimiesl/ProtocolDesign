/* Server Binary Protocol */
#include"serverb.h"

/* Handle signal */
void signalhandler(int signo){
	switch(signo){
	}
}

/* Create a unix socket and connect to server (for each client) */
int create_connect(){
	int ufd,length;
	struct sockaddr_un remote;
	
	if( (ufd = socket(AF_UNIX,SOCK_STREAM,0)) == -1){
		perror("socket, ");
		return -1;
	}

	memset(&remote,0,sizeof(struct sockaddr_un));
	remote.sun_family = AF_UNIX;	
	strcpy(remote.sun_path, SOCK_PATH);
	length = strlen(remote.sun_path) + sizeof(remote.sun_family);
	
	if( connect(ufd,(struct sockaddr*)&remote,length) == -1){
		perror("connect, ");
		close(ufd);
		return -1;	
	}
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
		if(state->ackreq){
			updateICP(&icp,0x00,0x00,0x01,0x01,0x00,0x00,0x0000,state->seq,state->rack);
			toBinary(&icp);
			memcpy(obuff,icp.buffer,8);
			sendto(sfd,(char*)obuff,8,0,&(state->addr),state->len);
			state->ackreq = false;
		}
		gettimeofday(&ct,NULL);
		list_for_each(idx,&(state->out.list)){
			queue = list_entry(idx,struct Queue,list);
			if( queue->sent && checktime(&(queue->st),&ct,500000) ){
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
	int temp; 
	int epollfd,sfd;
	int nfds,n;
	socklen_t len;
	size_t rsize,idx;
	size_t k;	
	struct ICP icp;
	struct sockaddr addr;
	struct epoll_event ev,events[1000];	
	struct State state_list;
	struct State * state;
	struct Queue * queue;
	struct timeval ct,pt;
	struct list_head * pos,*q;
	unsigned short s;	
	unsigned char ibuff[BUFF_SIZE];	
	unsigned char obuff[BUFF_SIZE];
	unsigned char fbuff[BUFF_SIZE];	
	bool cnt;
	
	idx = 0;
	len = sizeof(struct sockaddr);
	memset(&icp,0,sizeof(struct ICP));

	/* Create udp socket. AF_INET,AF_INET6 or AF_UNSPEC */	
	if( (sfd = custom_socket(AF_INET,argv[0])) == -1){
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
			continue;
		}
		for(n = 0; n < nfds; n++){
			if(events[n].data.fd == sfd){
				if( (rsize = recvfrom(sfd,(char*)ibuff,BUFF_SIZE,0,&addr,&len)) == -1){
					continue;
				}
				else if(rsize >= 8){
					memcpy(icp.buffer,ibuff,8);
					toValues(&icp);
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
							/* Send packet with start bit set */
							updateICP(&icp,0x01,0x00,0x01,0x01,0x00,0x00,
											0x0000,state->seq,state->rack);
							state->sentdown = state->seq;
							toBinary(&icp);
							printICPOut(&icp);
							memcpy(obuff,icp.buffer,8);
							/* Add to state memory */
							addOutPacketToState(state,obuff,icp.seq,0,0x00);
							list_add_tail(&(state->list),
											&(state_list.list));
							sendto(sfd,(char*)obuff,8,0,
											&(state->addr),state->len);								
						} 
					}
					else {
						if(icp.endbit == 0x01 && icp.size == 00){
						}
						else if(icp.ackbit == 0x01 && icp.size == 0){
							if(state->status == RC){
								if( (temp = create_connect()) == -1)
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
						}
						else if(icp.size != 0){
							/* Ack */
							ackThis(state,icp.ack,icp.cackbit,icp.cackbit);	
							if(!ackThat(state,icp.seq)){
								/* Immediate ack */
								updateICP(&icp,0x00,0x00,0x01,0x00,0x00,0x00,
													0x0000,state->seq,icp.seq);
								toBinary(&icp);
								printICPOut(&icp);
								memcpy(obuff,icp.buffer,8);
								sendto(sfd,(char*)obuff,8,0,&(state->addr),
											state->len);							
							}		
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
					}				
				}
			}
			else {
				if( (state = findState_fd(&state_list,events[n].data.fd)) != NULL){
					if( (rsize = recv(events[n].data.fd,(char *)ibuff,BUFF_SIZE,0)) > 0){
						for(k = 0;k <= (int)(rsize/1000);k++){
							(state->seq)=((state->seq)==65535)?1:(state->seq)+1;
							memcpy(obuff,ibuff+k*1000,(k==(int)(rsize/1000))?rsize%1000:1000);
							addOutPacketToState(state,obuff,icp.seq,icp.size,icp.frag);				
						}
						/* Send packet,if network is congestion free */ 
						sendPacket(state,sfd);
					}				
					else if(rsize == 0){
					}
				}				
			}	
		}
		gettimeofday(&ct,NULL);
		if(checktime(&pt,&ct,200000)){
			timeout(&state_list,sfd);
			gettimeofday(&pt,NULL);
		}
	}
	raise(SIGUSR1);
}
