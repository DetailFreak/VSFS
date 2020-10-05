
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <sys/wait.h>

#ifndef VSFS_CONFIG
#define VSFS_CONFIG

#define NUM_REPLICAS 3
#define MAX_DATA_SERVERS 512

/*
    Global Variables for messaging
*/
int msgid_c, msgid_m, msgid_d;
int addr_d[MAX_DATA_SERVERS];
int count_d;

/*
    Operation Codes 
*/
#define ADD_FILE 50
#define ADD_CHUNK 60
#define CHUNK_ADDRESSES 70
#define ADD_CHUNK_DATA 80
#define MOVE 90
#define COPY 100
#define COPY_CHUNK 110
#define ADD_CHUNK_DATA 120
#define REMOVE 130
#define DELETE_CHUNK 140
#define EXEC_CHUNK 140
#define EXEC_DATA 150
/*
    Ack Codes
*/
#define OK 200
#define ERROR 400
/*
    Common Message Buffer structures
*/
typedef struct mesg_buffer {
	long mtype; 
	char text[MAX_SIZE]; 
    char filepath[MAX_SIZE];
	char filename[MAX_SIZE];
	char chunkname[MAX_SIZE];
	long server_id;
} Message; 

typedef struct AddFileMessage {
    long mtype;
    char 
}
/*
    Safe variants of syscalls. For cleaner code
*/
key_t ftok_safe(const char* msgpath, int proj_id) {
    key_t ret;
    if((ret = ftok(msgpath, proj_id)) == -1){
		perror("ftok err");
		exit(1);
	} return ret;
}

int msgget_safe(key_t key, int msg_flag) {
    int ret;
    if((ret = msgget(key, msg_flag)) == -1) {
        perror("msgget");
		exit(1);
    } return ret;
}

char* getcwd_safe(char* buff, size_t siz) {
	if (getcwd(buff, siz) == NULL) {
		perror("getcwd");
		exit(1);
	} return buff;
}

/*
    Common init function for each server
*/
void init_mesg_queue(){
	char msgq_path[MAX_SIZE];
	getcwd_safe(msgq_path, MAX_SIZE);
	key_t key_c = ftok_safe(msgq_path, 'C'); 
	key_t key_m = ftok_safe(msgq_path, 'M');
    key_t key_d = ftok_safe(msgq_path, 'D');
	msgid_c = msgget_safe(key_c, 0666 | IPC_CREAT);
	msgid_m = msgget_safe(key_m, 0666 | IPC_CREAT);
	msgid_d = msgget_safe(key_d, 0666 | IPC_CREAT);
}

/*
    Utiilty functions used by multiple files
*/

char **parse_input(char *input, char *sep){
    /* 
    
    returns the list of commands (seperated by pipe in the input)
    
    input: user input from getline
    
    output: set of commands

    */
    char *tok;
    char **command = malloc(64 * sizeof(char *));
    int i = 0;

    tok = strtok(input, sep);

    while(tok != NULL){
        char *newline = strchr(tok, '\n');
        if(newline)
            *newline=0;
        command[i] = tok;
        // printf("%s\t",  command[i]);
        i++;
        tok = strtok(NULL, sep);
    }
    command[i] = NULL;
    return command;
}

#endif