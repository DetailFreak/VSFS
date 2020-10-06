// C Program for Message Queue (Writer Process) 
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <string.h>
#include <ctype.h>

#define MSGQ_PATH "/mnt/e/Sem3/NP/Assignment/client-server/"
#define MAX_SIZE 500
#define ADD_CHUNK 1

key_t key_cm;
key_t key_cd;
key_t key_md;
int msgid_cm;
int msgid_cd;
int msgid_md;


struct mesg_buffer {
    long mesg_type;  //for d_server, mesg_type is server ID
    char mesg_text[1024];
    char filename[128];
	int operation;
	int chunk_no;
	int server_id;
	int chunk_size;
} message; 


void init_mesg_queue(){

	if((key_cm = ftok(MSGQ_PATH, 'A')) == -1){
		perror("ftok err");
		exit(1);
	} 
	if((msgid_cm = msgget(key_cm, 0666 | IPC_CREAT)) == -1){
		perror("msgget cm");
		exit(1);
	}
	if((key_cd = ftok(MSGQ_PATH, 'B')) == -1){
		perror("ftok err");
		exit(1);
	} 
	if((msgid_cd = msgget(key_cd, 0666 | IPC_CREAT)) == -1){
		perror("msgget cd");
		exit(1);
	}
	if((key_md = ftok(MSGQ_PATH, 'C')) == -1){
		perror("ftok err md");
		exit(1);
	} 
	if((msgid_md = msgget(key_md, 0666 | IPC_CREAT)) == -1){
		perror("msgget");
		exit(1);
	}
}

void add_chunk(char *server){
	printf("Data %s \n", message.mesg_text);

	FILE *fp;
	char filename[256];
	strcpy(filename, message.filename);

	snprintf(filename, 256, "%s/server_%ld_%s_chunk_%d.txt", server, message.mesg_type, message.filename, message.chunk_no);

	fp = fopen(filename,"w");
	fwrite(message.mesg_text, message.chunk_size, 1, fp);
	fclose(fp);
}


int main(int argc, char** argv)
{
	printf("init..\n");
	init_mesg_queue();
	printf("%d \n", msgid_cd);
	long server_id = atol(argv[1]);

    // msgctl(msgid_cd, IPC_RMID, NULL);
	do
	{
		if( msgrcv(msgid_cd, &message, sizeof(message), server_id, IPC_NOWAIT) > 0){
			if(message.operation == ADD_CHUNK){
				char servername[256];
				snprintf(servername, 256, "server_%ld", server_id);
				add_chunk(servername);
			}
		}
	} while (1);

	return 0; 
} 
