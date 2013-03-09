#include"msgqueue.h"

int setup(){
	key_t key;
	int msqid;
	
	if( (key = ftok(PATH,ID)) == -1){
		perror("Error: ftok,");
		return -1;	
	}
	if( (msqid = msgget(key, 0666 | IPC_CREAT)) == -1){
		perror("Error: msgget,");
		return -1;
	}
	return msqid;
}

int join(){
	key_t key;
	int msqid;

	if( (key = ftok(PATH,ID))== -1){
		perror("Error:ftok,");
		return -1;
	}
	if( (msqid = msgget(key,0666)) == -1){
		perror("Error: msgget,");
		return -1;
	}
	return msqid;

}

int teardown(int msqid){
	int err;
	if( (err = msgctl(msqid,IPC_RMID,NULL)) == -1){
		perror("Error: msgctl,");
		return -1;	
	}
	return err;
}

int establish(int msqid,const char ip[],int port){
	int err;
	struct msgbuf_establish data;
	data.mtype = 1;
	strncpy(data.address.ip,ip,strlen(ip)+1);
	data.address.port = port;
	if( (err = msgsnd(msqid,&data,sizeof(struct msgbuf_establish)- sizeof(long),0)) == -1){
		perror("Error: msgsnd,");
		return -1;	
	}
	return 0;
}


