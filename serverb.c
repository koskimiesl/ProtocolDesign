/*Server Binary Protocol */

#include"serverb.h"
#include<sys/epoll.h>

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
	int epollfd,nfds,n;
	struct epoll_event ev,events[1000];
	socklen_t len;
	size_t rsize;
	struct State * tmp;
	struct ICP icp;
	pthread_t thread;
	struct sockaddr addr;
	struct timespec t;
	unsigned short s;	
	unsigned char ibuff[BUFF_SIZE];	
	unsigned char obuff[BUFF_SIZE];
	struct State state_list;
	struct State * state;
	struct Queue * queue;
	struct timeval ct;
	struct list_head * pos,*idx,*q;	
	int sfd;
	bool cnt;
	size_t k;
	
	len = sizeof(addr);
	
	if( (sfd = custom_socket(AF_INET,argv[0])) == -1) //AF_INET,AF_INET6 or AF_UNSPEC
		return -1;
	
	if( (epollfd = epoll_create(2)) == -1){
		perror("epoll_create");
		return -1;
	}

	// add ufd to epoll
	ev.events = EPOLLIN;
	ev.data.fd = sfd;
	if(epoll_ctl(epollfd,EPOLL_CTL_ADD,sfd,&ev) == -1){
		perror("epoll_ctl,sfd");
		return -1;	
	}

	INIT_LIST_HEAD(&state_list.list);
	while(1){
		if( (nfds = epoll_wait(epollfd,events,1000,200)) == -1){
			perror("epoll_wait");
			return -1;		
		}
		else if(nfds == 0){
			list_for_each(pos,&(state_list.list)){
				state = list_entry(pos,struct State,list);
				if(state->ackreq){
					icp.version = 0x01;
					icp.startbit = 0x00;
					icp.endbit = 0x00;
					icp.ackbit = 0x00;
					icp.cackbit = 0x00;
					icp.kalive = 0x00;
					icp.frag = 0x00;
					icp.size = 0x00;
					icp.seq = state->seq;
					icp.ack = state->rack;
					toBinary(&icp);
					memcpy(obuff,icp.buffer,8);
					sendto(sfd,(char*)obuff,8,0,&(state->addr),state->len);
					state->ackreq = false;
				}
				//gettimeofday(&ct,NULL);
				//list_for_each(idx,&(state->out.list)){
				//	queue = list_entry(idx,struct Queue,list);
				//	if( ((ct.tv_sec - queue->st.tv_sec) > 1 )	|| ( ((ct.tv_sec - queue->st.tv_sec) >= 0 ) && ((ct.tv_usec - queue->st.tv_sec) > 500000) ) ){
				//	sendto(sfd,(char *)queue->buffer,queue->size,0,&(state->addr),state->len);				
				//	} 				
				//}
			}
		}
		for(n = 0; n < nfds; n++){
			
			if(events[n].data.fd == sfd){
				if( (rsize = recvfrom(sfd,(char*)ibuff,BUFF_SIZE,0,&addr,&len)) == -1){
					continue;
				}
				else if(rsize >= 8){
					memcpy(icp.buffer,ibuff,8);
					toValues(&icp);
					printf("--> %d %d %d %d %d\n",(int)(icp.ackbit),(int)(icp.cackbit),icp.size,icp.seq,icp.ack); 
					if(icp.startbit == 0x01 && icp.size == 0){
						tmp = (struct State *)malloc(sizeof(struct State));
						initState(tmp);
						memcpy(&(tmp->addr),&addr,len);
						tmp->len = len;
						tmp->rack = icp.seq;
						tmp->sentup = icp.seq;
						// update
						icp.startbit = 0x01;
						icp.endbit = 0x00;
						icp.ackbit = 0x01;
						icp.cackbit = 0x01;
						icp.kalive = 0x00;
						icp.frag = 0x00;
						icp.size = 0x0000;
						icp.seq = tmp->seq;
						icp.ack = tmp->rack;
						toBinary(&icp);
						printf("<-- %d %d %d %d %d\n",(int)(icp.ackbit),(int)(icp.cackbit),icp.size,icp.seq,icp.ack); 
						memcpy(obuff,icp.buffer,8);
						// add to state memory
						addOutPacketToState(tmp,obuff,icp.seq,8);
						list_add_tail(&(tmp->list),&(state_list.list));
						sendto(sfd,(char *)obuff,8,0,&addr,len);
					}
					else if(icp.endbit == 0x01 && icp.size == 0){
					}
					else if(icp.ackbit == 0x01 && icp.size == 0){
						if((tmp = findState_addr(&state_list,&addr)) != NULL){
							if(tmp->status == RC){
								temp = create_connect();
								tmp->fd = temp;
								tmp->status = CT;
								// add ufd to epoll
								ev.events = EPOLLIN;
								ev.data.fd = temp;
								if(epoll_ctl(epollfd,EPOLL_CTL_ADD,temp,&ev) == -1){
									perror("epoll_ctl,temp");
									return -1;	
								}
							}
						}						
					}
					else if(icp.kalive = 0x01 && icp.size == 0){
					}
					else if(icp.size != 0){
						if((tmp = findState_addr(&state_list,&addr)) != NULL){
							// Ack
							ackThis(tmp,icp.ack,icp.ackbit,icp.cackbit);
							if(!ackThat(tmp,icp.seq)){
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
								icp.seq = tmp->seq;
								toBinary(&icp);
								printf("<-- %d %d %d %d %d\n",(int)(icp.ackbit),(int)(icp.cackbit),icp.size,icp.seq,icp.ack); 
								memcpy(obuff,icp.buffer,8);
								sendto(sfd,(char*)obuff,8,0,&(tmp->addr),tmp->len);
							}
							s = ((tmp->sentup)==65535)?1:(tmp->sentup)+1;
							if(s == icp.seq){
								tmp->sentup = s;
								memmove(obuff,ibuff+8,icp.size);
								send(tmp->fd,obuff,icp.size,0);
								s = ((tmp->sentup)==65535)?1:(tmp->sentup)+1;
								while(1){
									cnt = false;
									s = ((tmp->sentup)==65535)?1:(tmp->sentup)+1;
									list_for_each_safe(pos,q,&(tmp->in.list)){
										queue = list_entry(pos,struct Queue,list);
										if(queue->seq == s){
											tmp->sentup = s;
											send(tmp->fd,queue->buffer,queue->size,0);
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
							else {
								memmove(obuff,ibuff+8,icp.size);
								addInPacketToState(tmp,obuff,icp.seq,icp.size,icp.frag);
							}							
						}
					}
				}
			}
			else {
				if((tmp = findState_fd(&state_list,events[n].data.fd)) != NULL){
					if( (rsize = recv(events[n].data.fd,(char *)ibuff,BUFF_SIZE,0)) > 0){
						if(rsize <= 1000){
							// Update seq
							tmp->seq=(tmp->seq==65535)?1:tmp->seq+1;
							// Update
							icp.version = 0x01;
							icp.startbit = 0x00;
							icp.endbit = 0x00;
							if(tmp->ackreq){
								icp.ackbit = 0x01;
								icp.cackbit = 0x01;
								tmp->ackreq = false;
							}
							else {
								icp.ackbit = 0x00;
								icp.cackbit = 0x00;
							}
							icp.kalive = 0x00;
							icp.frag = 0x00;						
							icp.size = rsize;
							icp.seq = tmp->seq;
							icp.ack = tmp->rack;
							toBinary(&icp);
							printf("<-- %d %d %d %d %d\n",(int)(icp.ackbit),(int)(icp.cackbit),icp.size,icp.seq,icp.ack); 
							memcpy(obuff,icp.buffer,8);
							memcpy(obuff+8,ibuff,rsize);
							sendto(sfd,(char*)obuff,8+rsize,0,&(tmp->addr),tmp->len);
							// add to state memory
							addOutPacketToState(tmp,obuff,icp.seq,8+rsize);						
						}
						else {
							for(k = 0;k <= (int)(rsize/1000);k++){
								// Update seq
								tmp->seq=((tmp->seq)==65535)?1:(tmp->seq)+1;
								// UPdate
								icp.version = 0x01;
								icp.startbit = 0x00;
								icp.endbit = 0x00;
								if(tmp->ackreq){
									icp.ackbit = 0x01;
									icp.cackbit = 0x01;
									tmp->ackreq = false;
								}
								else {
									icp.ackbit = 0x00;
									icp.cackbit = 0x00;								
								}
								icp.kalive = 0x00;
								icp.frag = (k==(int)(rsize/1000))?0x00:0x01;
								icp.size = (k==(int)(rsize/1000))?rsize%1000:1000;
								icp.seq = tmp->seq;
								icp.ack = tmp->rack;
								toBinary(&icp);
								printf("<-- %d %d %d %d %d %d\n",(int)(icp.ackbit),(int)(icp.cackbit),(int)(icp.frag),icp.size,icp.seq,icp.ack);
								memcpy(obuff,icp.buffer,8);
								memcpy(obuff+8,ibuff+(k*1000),icp.size);
								sendto(sfd,(char*)obuff,8+icp.size,0,&(tmp->addr),tmp->len);
								// add to state memory
								addOutPacketToState(tmp,obuff,icp.seq,8+icp.size);				
							}
						}				
					}
					else if(rsize == 0){
					}
				}
			}
			
		}
	}
	return 0;
}
