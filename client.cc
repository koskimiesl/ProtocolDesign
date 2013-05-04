/* IoTPS Client */
#include"client.hh"

int connect(){
	int ufd;
	int length;
	struct sockaddr_un remote;
	if( (ufd = socket(AF_UNIX,SOCK_STREAM,0)) == -1){
		perror("socket");
		return -1;	
	}
	memset(&remote,0,sizeof(struct sockaddr_un));
	remote.sun_family = AF_UNIX;	
	strcpy(remote.sun_path, SOCK_PATH);
	length = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if( connect(ufd,(struct sockaddr*)&remote,length) == -1)
		perror("connect");
	return ufd;
}

int main(int argc,char *argv[]){
	char ip[IPLEN],port[PORTLEN];
	char buff[BUFF_SIZE];
	char tbuff[BUFF_SIZE];
	char * p;
	CommMessage text;	
	
	pid_t pid;
	std::string cmd;
	std::vector<std::string> list;
	std::vector<std::string> slist;
	std::vector<std::string> ulist;	
	enum REQ req;
	fd_set rfds;	
	int opt;
	int ch;
	int ret;
	int ufd;
	size_t rsize;
	struct timespec t;
	t.tv_sec = 1;
	t.tv_nsec = 0;
	ch = 0;
	req = NONE;

	// Parse command line options
	while( (opt = getopt(argc,argv, "s:p:")) != -1){
		switch(opt){
			case 's':
				strncpy(ip,optarg,IPLEN);
				break;
			case 'p':
				strncpy(port,optarg,PORTLEN);
				break;
			case '?':
				return -1;
			default:
				return -1;		
		}
	}

	if( (pid = fork()) < 0){
		perror("fork");
		return -1;
	}
	else if(pid == 0){
		if(execl("clientb",ip,port,(void *)0) == -1){
			perror("execl");
			return -1;		
		}
	}
	
	int fd;
	fd = open("temp1.txt",O_CREAT | O_WRONLY,0777);
	
	sleep(5);
	if( (ufd = connect()) == -1)
		return -1;
	// kill process
	// ncurses
	Screen scr;
	while(1){
		FD_ZERO(&rfds);
		FD_SET(ufd,&rfds);
		FD_SET(0,&rfds);
		if( (ret = pselect(ufd+1,&rfds,NULL,NULL,&t,NULL)) == -1){
			continue;
		}
		else if(FD_ISSET(0,&rfds)){
			scr.status("kk");
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
				close(ufd);
				close(fd);
				kill(pid,SIGINT);//capture and clear all 
				return 0;		
			}
			else if(ch == 'R' || ch == 'r'){ //refresh,get sensors list	
				text.updateClientID("abc123");
				std::string msg = text.createListRequest();			
				memcpy(buff,msg.c_str(),msg.size());
				send(ufd,buff,msg.size(),0);
				scr.status("List request sent.");
				req = LIST;
			}
			else if(ch == 'S' || ch == 's'){
				text.updateClientID("abc123");
				slist = scr.getSList();
				text.updateDeviceIDs(slist);
				text.updateCount(slist.size());
				std::string msg = text.createSubsRequest();
				memcpy(buff,msg.c_str(),msg.size());
				send(ufd,buff,msg.size(),0);
				scr.status("Subs request sent.");
				req = SUBS;
			}	
			else if(ch == 'U' || ch == 'u'){
				text.updateClientID("abc123");
				ulist = scr.getUList();
				text.updateDeviceIDs(ulist);
				text.updateCount(ulist.size());
				std::string msg = text.createUnsubsRequest();
				memcpy(buff,msg.c_str(),msg.size());
				send(ufd,buff,msg.size(),0);
				scr.status("Unsubs request sent.");
				req = UNSUBS;
			}
			ch = 0;
		}
		else if(FD_ISSET(ufd,&rfds)){
			//read
			rsize = recv(ufd,buff,BUFF_SIZE,0);
			p = strstr(buff,"\r\n\r\n");
			memcpy(tbuff,buff,p-buff+1);
			text.updateMessage(tbuff);
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
			else if(cmd == "UPDATES"){
				//TODO
				memcpy(tbuff,p+2,text.getSize());
				// only test,make sensor specific file
				write(fd,tbuff,text.getSize());
				scr.status("Updates.");
				text.print();
				req = NONE;			
			}
		}			
	}
	return 0;
}
