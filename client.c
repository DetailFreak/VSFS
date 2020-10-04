// C Program for Message Queue (Writer Process) 
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <string.h>


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
	message.mesg_type = 50; 
}


void add_chunks(){
	msgrcv(msgid, &message, sizeof(message), 50, 0);
}

void test(){
	
}


int main() 
{
	key_t key; 
	int msgid; 
	

	printf("add a file : "); 
	fgets(message.mesg_text, 1000, stdin);
	msgsnd(msgid, &message, sizeof(message), 0); 

	printf("Data send is : %s \n", message.mesg_text); 

	return 0; 
} 
