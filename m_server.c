#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_NAME_LEN 64
#define ADDRESS_LEN 128
#define MAX_CHILDREN 512
#define FS_FILE 1
#define FS_DIR 0

typedef struct Chunk {
    char name[MAX_NAME_LEN];
    char node[3][ADDRESS_LEN];
} Chunk;

typedef struct FileMeta {
    int num_chunks;
    Chunk *chunks;
} FileMeta;

typedef struct Node {
    int type;
    int num_children;
    char name[MAX_NAME_LEN];
    FileMeta* meta;
    struct Node *child[MAX_CHILDREN];
} Node;

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

Node* mknode(int type, const char* name) {
    Node* node = malloc(sizeof(Node));
    node->type = type;
    node->meta = (type == FS_FILE) ? (malloc(sizeof(FileMeta))) : (NULL) ;
    strcpy(node->name, name);
    node->num_children = 0;
    for(int i = 0; i<MAX_CHILDREN; ++i)
        node->child[i] = NULL;
    return node;
}

const char* type2str(int type){
    if (type == FS_FILE) return "file";
    return "dir";
}

Node* find_child(Node *fs, const char* name){
    for(int i = 0; i < fs->num_children; ++i)
        if (strcmp(fs->child[i]->name, name) == 0)
            return fs->child[i];
    return NULL;
}

int insert_child(Node* fs, int type, const char* name) {
    if (fs->num_children == MAX_CHILDREN){
        perror("Error inserting: Node limit reached!\n");
        return -1;
    }
    if (find_child(fs, name)){
        perror("Error inserting: Node with same name alread exists.\n");
        return -1;
    }
    if (fs->type == FS_FILE){
        perror("Error inserting: files cannot have any child nodes.\n");
        return -1;
    }
    fs->child[fs->num_children++] = mknode(type, name);
    return 0;
}

int find_child_idx(Node* fs, const char* name){
    for(int i = 0; i<fs->num_children; ++i)
        if (strcmp(fs->child[i]->name,name) == 0)
            return i;
    return -1;
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

void print_children(Node* fs) {
    for(int i = 0; i<fs->num_children; ++i)
        printf("%s (%s)\t",fs->child[i]->name, type2str(fs->child[i]->type));
    printf("\n");
}

Node* find_node(Node* fs, const char* pth) {
    
    char *path = malloc(sizeof(pth));
    strcpy(path, pth);
    
    int i;
    Node *temp = fs;
    char **path_nodes = parse_input(path, "/");
    
    while(*path_nodes){
        i = find_child_idx(temp, *path_nodes);
        if (i == -1) {
            perror("%s is not a file or directory.\n");
            return NULL;
        }
        temp = temp->child[i];
        ++path_nodes;
    }
    return temp;
}

int insert_node(Node* fs, const char* pth, int type, const char* name) {
    char *path = malloc(sizeof(pth));
    strcpy(path, pth);
    
    Node* parent;
    if ((parent  = find_node(fs, path)) == NULL) {
        perror("Error insert_node: invalid path\n");
        return -1;
    }
    if (insert_child(parent, type, name) == -1) {
        perror("Error insert_node: insert child failed\n");
        return -1;
    }
    return 0;
}

int delete_node(Node* fs, const char* pth, int type, const char* name) {
    char *path = malloc(sizeof(pth));
    strcpy(path, pth);
    
    Node* parent;
    if ((parent  = find_node(fs, path)) == NULL) {
        perror("Error delete_node: invalid path\n");
        return -1;
    }
    if (delete_child(parent, name) == -1) {
        perror("Error delete_node: insert child failed\n");
        return -1;
    }
    return 0;
}

char ** divide_path(const char* path) {
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

void printfs(Node* fs) {
    if (fs->type == FS_DIR && fs->num_children){
    printf("%s: ",fs->name);
    print_children(fs);
    for(int i=0; i<fs->num_children; ++i){
        printfs(fs->child[i]);
    }
    }
}

int main() {
    Node* fs = mknode(FS_DIR, "root");
    insert_child(fs, FS_DIR, "music");
    insert_child(fs, FS_DIR, "movies");
    insert_child(fs, FS_DIR, "games");
    insert_child(fs, FS_FILE, "index.html");
    // delete_child(fs, "index.html");
    insert_node(fs, "games", FS_DIR, "dmc");
    insert_node(fs, "/games/dmc", FS_DIR,"bin");
    printfs(fs);
    char path[128];
    strcpy(path,"/hello/file.txt/");
    char** parts = parse_input(path, "/");
    printf("%s\n", parts[1]);
    exit(EXIT_SUCCESS);
}
