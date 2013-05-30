/* IoTPS Client */

#include <fstream>
#include <unistd.h>

#include"client.hh"
#include"helpers.hh"

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
	char ip[IPLEN],port[PORTLEN],id[IDLEN],lossp[LOSSLEN],lossq[LOSSLEN];
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
	int count = 0;
	struct timespec t;
	t.tv_sec = 1;
	t.tv_nsec = 0;
	ch = 0;
	req = NONE;

	// Parse command line options
	while( (opt = getopt(argc,argv, "s:p:i:P:Q:")) != -1){
		switch(opt){
			case 's':
				strncpy(ip,optarg,IPLEN);
				break;
			case 'p':
				strncpy(port,optarg,PORTLEN);
				break;
			case 'i':
				strncpy(id,optarg,IDLEN);
				break;
			case 'P':
				strncpy(lossp,optarg,LOSSLEN);
				break;
			case 'Q':
				strncpy(lossq,optarg,LOSSLEN);
				break;
			case '?':
				return -1;
			default:
				return -1;		
		}
	}

	// create directory for log files
	std::string clientid(id);
	std::string dirname = "client_" + clientid + "_logs";
	if (createDir(dirname) == -1)
		return -1;

	if( (pid = fork()) < 0){
		perror("fork");
		return -1;
	}
	else if(pid == 0){
		if(execl("clientb",ip,port,lossp,lossq,(void *)0) == -1){
			perror("execl");
			return -1;		
		}
	}
	
	sleep(5);
	if( (ufd = connect()) == -1)
		return -1;
	std::stringstream mm;
	mm << setngetR(ufd);
	// kill process
	// ncurses
	Screen scr;
	scr.status(mm.str().c_str());
	while(1){
		FD_ZERO(&rfds);
		FD_SET(ufd,&rfds);
		FD_SET(0,&rfds);
		if( (ret = pselect(ufd+1,&rfds,NULL,NULL,&t,NULL)) == -1){
			continue;
		}
		else if(FD_ISSET(0,&rfds)){
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
				scr.status("Wait,Exiting...");
				close(ufd);
				//kill(pid,SIGINT);//capture and clear all 
				sleep(4);				
				return 0;		
			}
			else if(ch == 'R' || ch == 'r'){ //refresh,get sensors list	
				text.updateClientID(clientid);
				std::string msg = text.createListRequest();			
				memcpy(buff,msg.c_str(),msg.size());
				send(ufd,buff,msg.size(),0);
				scr.status("List request sent.");
				req = LIST;
			}
			else if(ch == 'S' || ch == 's'){
				text.updateClientID(clientid);
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
				text.updateClientID(clientid);
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
			std::stringstream mm;
			mm << count;
			scr.status(mm.str().c_str());
			count++;			
			if (rsize == 0)
			{
				if (close(ufd) == -1)
				{
					perror("close");
					return -1;
				}
				return 0;
			}
			p = strstr(buff,"\r\n\r\n");
			memset(tbuff, 0, BUFF_SIZE); // clear previous messages
			memcpy(tbuff,buff,p-buff+1);
			text.updateMessage(tbuff);
			text.parse();
			cmd = text.getCommand();
			if(cmd == "OK" && req == LIST){
				list = text.getDeviceIDs();
				if(list.size() == 0)
					scr.status("No sensors to subscribe");
				else{
					scr.addAList(list);
					scr.status("Press 'R' to retrive list, 'S' to subscribe and 'U' to unsubscribe.");
				}
				req = NONE;
			}
			else if(cmd == "OK" && req == SUBS){
				list = text.getDeviceIDs();
				scr.addSList(list);
				scr.status("Press 'R' to retrive list, 'S' to subscribe and 'U' to unsubscribe.");
				req = NONE;			
			}
			else if(cmd == "OK" && req == UNSUBS){
				list = text.getDeviceIDs();
				std::vector<std::string> prevList = scr.getPList();
				std::vector<std::string>::const_iterator it;
				for (it = list.begin(); it != list.end(); it++)
				{
					std::vector<std::string>::iterator itr = std::find(prevList.begin(), prevList.end(), *it);
					if (itr != prevList.end())
						prevList.erase(itr);
					else
						std::cerr << "client was not previously subscribed to this sensor" << std::endl;
				}
				scr.addSList(prevList);
				//scr.status("Press 'R' to retrive list, 'S' to subscribe and 'U' to unsubscribe.");
				req = NONE;
			}
			else if(cmd == "UPDATES"){	
				memcpy(tbuff,p+4,text.getSize());
				std::vector<std::string> t = text.getDeviceIDs();
				std::string binarytest(tbuff, 9);
				for(std::vector<std::string>::iterator itr = t.begin();itr != t.end();itr++)
					if (itr->find("camera") != std::string::npos && binarytest != "NO_MOTION")
						logClientIncoming(dirname, *itr, tbuff, text.getSize(), text.getTimeStamp(), getTimeStamp(), true);
					else
						logClientIncoming(dirname, *itr, tbuff, text.getSize(), text.getTimeStamp(), getTimeStamp(), false);
				req = NONE;
			}
		}
	}
	return 0;
}
