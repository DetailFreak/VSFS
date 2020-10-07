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
#define RM_CHUNK 2

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

	snprintf(filename, 256, "%s/server_%ld_%s_chunk_%d.txt", server, message.mesg_type, message.filename, message.chunk_no);

	fp = fopen(filename,"w");
	fwrite(message.mesg_text, message.chunk_size, 1, fp);
	fclose(fp);
}


void remove_chunk(char *server){
	char filename[256];
	int chunk_num = 1;
	message.mesg_type = 3;

	// puts("inside remove chunk");

	puts(filename);
	snprintf(filename, 256, "%s/server_%ld_openflow_Group_select_chunk_%d.txt", server, message.mesg_type, chunk_num);

	while( access(filename, F_OK ) != -1 ) {
    	// file exists
		if(remove(filename) == 0){
			printf("--------- file openflow_Group_select chunk %d removed \n", chunk_num);	
		}else{
			printf("--------- Unable to deelete file %s chunk %d \n", message.filename, chunk_num);	
		}
		snprintf(filename, 256, "%s/server_%ld_openflow_Group_select_chunk_%d.txt", server, message.mesg_type, ++chunk_num); 
	} 

}


void send_chunk(long receiver_id, char *filename, int chunk_num, long server_id, char *server_name){
	FILE *file = NULL;
    size_t bytesRead = 0;
	char srcfile[128];

	message.operation = 1;
	message.mesg_type = receiver_id;
	message.chunk_no = chunk_num;
	
	strcpy(message.filename, filename);
	snprintf(srcfile, 128, "%s/server_%ld_openflow_Group_select_chunk_%d.txt", server_name, server_id, chunk_num);

	file = fopen(srcfile, "r");
	if(file != NULL){
		while ((bytesRead = fread(message.mesg_text, 1, 128, file)) > 0){
			printf("----  %s ----- \n", message.mesg_text);
			if(msgsnd(msgid_cd, &message, sizeof(message), 0) == -1){
				perror("add chunk send error");
			}
		}
		fclose(file);
	}
}



int main(int argc, char** argv)
{
	printf("init..\n");
	init_mesg_queue();
	printf("%d \n", msgid_cd);
	long server_id = atol(argv[1]);

    // msgctl(msgid_cd, IPC_RMID, NULL);
	// remove_chunk("server_3");
	if(server_id == 1)
		send_chunk(2, "openflow_Group_select", 4, server_id, "server_1");
	do
	{
		if( msgrcv(msgid_cd, &message, sizeof(message), server_id, IPC_NOWAIT) > 0){
			if(message.operation == ADD_CHUNK){
				char servername[256];
				snprintf(servername, 256, "server_%ld", server_id);
				add_chunk(servername);
			}
			if(message.operation == RM_CHUNK){
				char servername[256];
				snprintf(servername, 256, "server_%ld", server_id);
				remove_chunk(servername);
			}
		}
	} while (1);

	return 0; 
} 
