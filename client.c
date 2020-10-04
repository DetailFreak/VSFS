// C Program for Message Queue (Writer Process) 
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <string.h>


#define MSGQ_PATH "/mnt/e/Sem3/NP/Assignment/client-server/"
#define MAX_SIZE 1000

typedef struct Chunk {
    char name[MAX_NAME_LEN];
    char node[3][ADDRESS_LEN];
} Chunk;


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


void add_chunks(int msgid, char* chunkname, int chunksize, int filesize, char* filepath){
	//msgrcv(msgid, &message, sizeof(message), 50, 0);
	int chunks = filesize/chunksize;
	for(int i = 0; i<chunks; ++i) {
		msgsnd(msgid, &message, sizeof(message), 70);
	}
	printf("sent chunks");
	for(int i = 0; i<chunks; ++i) {
		//msgrcv(msgid, &message, 200);
	}
}

void add_file(){
	msgrcv(msgid, &message, sizeof(message), 50, 0);
	Node* temp = fs;
    char path_cpy[256];
    strcpy(path_cpy, path);
    char ** path_comp = parse_input(path_cpy, "/");
    while(*path_comp) {
        Node* child;
        if ((child = find_child(temp, *path_comp)) && child->type == FS_DIR) {
            ;
        } else {
            insert_child(temp, FS_DIR, *path_comp);
            child = temp->child[temp->num_children-1];
        }
        temp = child;
        ++path_comp;
    }
    return temp;
}

void delete_file() {
	Node* node = malloc(sizeof(Node));
    node->type = type;
    node->meta = (type == FS_FILE) ? (malloc(sizeof(FileMeta))) : (NULL) ;
    strcpy(node->name, name);
    node->num_children = 0;
    for(int i = 0; i<MAX_CHILDREN; ++i)
        node->child[i] = NULL;
    return node;
}

int main() 
{
	key_t key; 
	int msgid; 
	

	printf("add a file : "); 
	while(1){
		fgets(message.mesg_text, 1000, stdin);
		if (strcmp(message.mesg_text, "exit") == 0)
			exit(EXIT_SUCCESS);
	}
	msgsnd(msgid, &message, sizeof(message), 0); 

	printf("Data send is : %s \n", message.mesg_text); 

	return 0; 
} 
