#include <stdio.h> 
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 

#define MSGQ_PATH "/mnt/e/Sem3/NP/Assignment/client-server/"
#define MAX_SIZE 500


struct mesg_buffer {
	long mesg_type; 
	char mesg_text[MAX_SIZE];
	char mesg_filename[MAX_SIZE];
	char mesg_chuckname[MAX_SIZE];
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
	msgrcv(msgid, &message, sizeof(message), 60, 0);
	if (overwrite == 0) 
        return insert_node(fs, path, FS_FILE, file_name);
    
    Node* dir = create_path(fs, path);
    return insert_child(dir, FS_FILE, file_name); 
}


void move(char *src, char *dest){
	msgrcv(msgid, &message, sizeof(message), 70, 0);
	char **parts = malloc(2*sizeof(char*));
    for(int i = strlen(path)-1; i >= 0; --i) {
        if (path[i] == '/' && i != strlen(path)-1) {
            parts[0] = malloc(i+1);
            for(int j = 0; j < i; ++j) {
                parts[0][j] = path[j];
            } parts[0][i] = '\0';

            parts[1] = malloc(strlen(path) - i);
            for(int j = 0; j < strlen(path) - i - 1 && path[i+j+1] != '/'; ++j) {
                parts[1][j] = path[i+j+1];
            } parts[1][strlen(path) - i] = '\0';
            
            return parts;
        }
    }
    return NULL;
}


void copy(char *src, char *dest){
	msgrcv(msgid, &message, sizeof(message), 70, 0);
	char **parts = malloc(2*sizeof(char*));
    for(int i = strlen(path)-1; i >= 0; --i) {
        if (path[i] == '/' && i != strlen(path)-1) {
            parts[0] = malloc(i+1);
            for(int j = 0; j < i; ++j) {
                parts[0][j] = path[j];
            } parts[0][i] = '\0';

            parts[1] = malloc(strlen(path) - i);
            for(int j = 0; j < strlen(path) - i - 1 && path[i+j+1] != '/'; ++j) {
                parts[1][j] = path[i+j+1];
            } parts[1][strlen(path) - i] = '\0';
            
            return parts;
        }
    }
    return NULL;
}


int delete_child(Node *fs, const char* name) {
    int i, j;
    if ((i = find_child_idx(fs, name)) == -1){
        perror("Error deleting, Node not found!.\n");
        return -1;
    }

    if (fs->child[i]->type == FS_FILE){
        free(fs->child[i]->meta);
    } free(fs->child[i]);

    for(j = i+1; j<fs->num_children; ++j){
        fs->child[j-1] = fs->child[j];
    } fs->child[--(fs->num_children)] = NULL;

    return 0;
}


int main() 
{
	init_mesg_queue();
	msgrcv(msgid, &message, sizeof(message), 50, 0); 
	printf(" %s \n", message.mesg_text); 
	msgctl(msgid, IPC_RMID, NULL); 
	return 0; 
} 
