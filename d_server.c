#include <stdio.h> 
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 

#define MSGQ_PATH "/mnt/e/Sem3/NP/Assignment/client-server/"
#define MAX_SIZE 1000


// structure for message queue 
struct mesg_buffer {
	long mesg_type; 
	char mesg_text[MAX_SIZE]; 
	long server_id;
} message; 


void init_mesg_queue(){
	key_t keycm; 
	key_t keycd; 
	key_t keymd; 
	int msgid; 

	if((keycm = ftok(MSGQ_PATH, 'A')) == -1){
		perror("ftok err");
		exit(1);
	} 
	if((msgid = msgget(key, 0666 | IPC_CREAT)) == -1){
		perror("msgget");
		exit(1);
	}
	if((keycd = ftok(MSGQ_PATH, 'B')) == -1){
		perror("ftok err");
		exit(1);
	} 
	if((msgid = msgget(key, 0666 | IPC_CREAT)) == -1){
		perror("msgget");
		exit(1);
	}
	if((keymd = ftok(MSGQ_PATH, 'C')) == -1){
		perror("ftok err");
		exit(1);
	} 
	if((msgid = msgget(key, 0666 | IPC_CREAT)) == -1){
		perror("msgget");
		exit(1);
	}
}

void add_chunks(){
	msgrcv(msgid, &message, sizeof(message), 50, 0);
}

int main() 
{ 
	msgrcv(msgid, &message, sizeof(message), 50, 0); 
	printf(" %s \n", message.mesg_text); 
	msgctl(msgid, IPC_RMID, NULL); 

	return 0; 
} 
