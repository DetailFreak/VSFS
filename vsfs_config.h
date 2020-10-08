
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
#define LIST_FILES 20
#define STOP_DSERVER 30
#define START_DSERVER 40
#define ADD_FILE 50
#define ADD_CHUNK 60
#define RM_CHUNK 70
#define CHUNK_ADDRESSES 80
#define DELETE_FILE 90
#define MOVE 100
#define COPY 110
#define COPY_CHUNK 120
#define REMOVE 130
#define DELETE_CHUNK 140
#define EXEC_CHUNK 150
#define EXEC_DATA 160
/*
    Ack Codes
*/
#define ACK 500
#define OK 1
#define ERROR 0
#define DEAD 2
/*
    Common Message Buffer structures
*/
#define MAX_SIZE 512

typedef struct mesg_buffer {
	long mtype; 
    int operation;
	char text[MAX_SIZE]; 
    char filepath[MAX_SIZE];
	char chunkname[MAX_SIZE];
    int addr_d[NUM_REPLICAS];
	long server_id;
    int msg_id;
    int chunk_size;
} Message; 

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
	key_t key_c = ftok_safe(msgq_path, 'X');
	key_t key_m = ftok_safe(msgq_path, 'M');
    key_t key_d = ftok_safe(msgq_path, 'D');
	msgid_c = msgget_safe(key_c, 0666 | IPC_CREAT);
	msgid_m = msgget_safe(key_m, 0666 | IPC_CREAT);
	msgid_d = msgget_safe(key_d, 0666 | IPC_CREAT);
    // printf("msg_c %d \n", msgid_c);
    // printf("msg_m %d \n", msgid_m);
    // printf("msg_d %d \n", msgid_d);
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

int async_recv(int msgid, Message* msg, int type) {
    if (msgrcv(msgid, msg, sizeof(Message), type, IPC_NOWAIT) != -1) 
	    return 1; 
    else return 0;
}

int sync_recv(int msgid, Message* msg, int type) {
    if (msgrcv(msgid, msg, sizeof(Message), type, 0) != -1) 
	    return 1; 
    else return 0;
}

int sync_send(int msgid, Message* msg) {
    if (msgsnd(msgid, msg, sizeof(Message), 0) != -1)
        return 1;
    else return 0;
    
}

int count_args(char ** args) {
    int count = 0;
    while(*args) {
        args++;
        count ++;
    } 
    return count;
}

#endif