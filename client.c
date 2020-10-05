#include "vsfs_config.h"

void add_chunks(int msgid, char* chunkname, int chunksize, int filesize, char* filepath){
	//msgrcv(msgid, &message, sizeof(message), 50, 0);
	// int chunks = filesize/chunksize;
	// for(int i = 0; i<chunks; ++i) {
	// 	msgsnd(msgid, &message, sizeof(message), 70);
	// }
	// printf("sent chunks");
	// for(int i = 0; i<chunks; ++i) {
	// 	//msgrcv(msgid, &message, 200);
	// }
}

void add_file(){
	// msgrcv(msgid, &message, sizeof(message), 50, 0);
	// Node* temp = fs;
    // char path_cpy[256];
    // strcpy(path_cpy, path);
    // char ** path_comp = parse_input(path_cpy, "/");
    // while(*path_comp) {
    //     Node* child;
    //     if ((child = find_child(temp, *path_comp)) && child->type == FS_DIR) {
    //         ;
    //     } else {
    //         insert_child(temp, FS_DIR, *path_comp);
    //         child = temp->child[temp->num_children-1];
    //     }
    //     temp = child;
    //     ++path_comp;
    // }
    // return temp;
}

void delete_file() {
	// Node* node = malloc(sizeof(Node));
    // node->type = type;
    // node->meta = (type == FS_FILE) ? (malloc(sizeof(FileMeta))) : (NULL) ;
    // strcpy(node->name, name);
    // node->num_children = 0;
    // for(int i = 0; i<MAX_CHILDREN; ++i)
    //     node->child[i] = NULL;
    // return node;
}

int main() 
{	init_mesg_queue();
	Message msg;
	msg.mtype = 1;
	while(1){
		fgets(msg.text, 1000, stdin);
		if (strcmp(msg.text, "exit") == 0)
			exit(EXIT_SUCCESS);
		msgsnd(msgid_m, &msg, sizeof(msg), 0); 
	}
	printf("Data sent is : %s \n", msg.text); 

	return 0; 
} 
