#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <string.h>
#include <ctype.h>

#define MSGQ_PATH "/mnt/e/Sem3/NP/Assignment/client-server/"
#define MAX_SIZE 1000
#define MAX_NAME_LEN 1000
#define ADDRESS_LEN 1000
#define ADD_CHUNK 1

key_t key_cm; 
key_t key_cd; 
key_t key_md; 
int msgid_cm;
int msgid_cd;
int msgid_md;


typedef struct Chunk {
    char name[MAX_NAME_LEN];
    char node[3][ADDRESS_LEN];
} Chunk;


// structure for message queue 
struct mesg_buffer { 
    long mesg_type;  // for d_server, mesg_type is server ID
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
		perror("ftok err");
		exit(1);
	} 
	if((msgid_md = msgget(key_md, 0666 | IPC_CREAT)) == -1){
		perror("msgget md");
		exit(1);
	}
}



void add_file(){

}

void delete_file() {

}

char* get_input(){
    char *buf;
    size_t size = 1024;
    buf = (char*) malloc(size);
    printf("$ ");
    if (getline(&buf, &size, stdin) == -1){
            exit(EXIT_FAILURE);
    }
    return buf;
}


char *remove_space(char *str){
    /*
        Removes whitespace from both ends of a string, 
        this function modifies the existing string.
    */
    int start = 0, i;
    int end = strlen(str) - 1;
    while (isspace((unsigned char) str[start])){
        start++;
    }
    while ((end >= start) && isspace((unsigned char) str[end])){
        end--;
    }
    for (i = start; i <= end; i++){
        str[i - start] = str[i];
    }
    str[i - start] = '\0';
    return str;
}



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


void add_chunk(){
	// printf(" %d \n", msgid_cd);
	// strcpy(message.mesg_text, "add");

	FILE *file = NULL;
    unsigned char buffer[1024];
    size_t bytesRead = 0;

	printf("Enter the chunk size: "); 
    scanf("%d", &message.chunk_size);

    // message.mesg_type = 70;
    message.operation = 1; 

	strcpy(message.filename,  "openflow_Group_select");


	for (int i = 1; i <= 3 ; i++)
	{
		message.chunk_no = 1;
		message.mesg_type = i;
		file = fopen("openflow_Group_select.txt", "r");   
		if (file != NULL){
			while ((bytesRead = fread(message.mesg_text, 1, message.chunk_size, file)) > 0){
				printf("-- %s -- %ld -- %d \n", message.mesg_text, message.mesg_type, msgid_cd);
				if(msgsnd(msgid_cd, &message, sizeof(message), 0) == -1){
					perror("add chunk send error");
				}
				message.chunk_no++;
			}
			fclose(file);
		}
	}
	
	 
}



void cmd_loop(){
	char *inp;
	char input[1000];
	char **command;


	do{
		printf(" %d \n", msgid_cd);
		inp = get_input();
		strcpy(input, inp);
		printf("=> %s", input);
		fflush(stdout);
		// if (strcmp(message.mesg_text, "exit") == 0)
		// exit(EXIT_SUCCESS);
		// command = parse_input(input, " ");
		// puts(command[1]);
		// if(command[1] == "add_chunk"){
		// 	//call add chunk
		// }

		add_chunk();
		free(inp);

	}while(1);


}


int main() 
{
	init_mesg_queue();
	cmd_loop();
	return 0;
} 
