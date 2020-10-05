#include "vsfs_config.h"

#define MAX_NAME_LEN 256
#define MAX_CHILDREN 512
#define FS_FILE 1
#define FS_DIR 0

#define INIT_BUFF_LEN = 100;
#define MAX_SIZE 500

typedef struct Chunk {
    char name[MAX_NAME_LEN];
    int addr[NUM_REPLICAS];
} Chunk;

typedef struct FileMeta {
    int num_chunks;
    size_t buff_size;
    Chunk *chunks;
} FileMeta;

typedef struct Node {
    int type;
    int num_children;
    char name[MAX_NAME_LEN];
    FileMeta* meta;
    struct Node *child[MAX_CHILDREN];
} Node;

void init_chunk(Chunk* chunk, const char* chunk_name, int * node_addresses) {
    strcpy(chunk->name, chunk_name);
    for(int i=0; i<NUM_REPLICAS; ++i) {
        chunk->addr[i] = node_addresses[i];
    }
}

FileMeta* mkmeta() {
    FileMeta* meta = malloc(sizeof(FileMeta));
    meta->chunks = NULL;
    meta->num_chunks = 0;
    meta->buff_size = 0;
    return meta;
}

Node* mknode(int type, const char* name) {
    Node* node = malloc(sizeof(Node));
    node->type = type;
    node->meta = (type == FS_FILE) ? (mkmeta()) : (NULL) ;
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
        printf("%s:",fs->name);
        print_children(fs);
        for(int i=0; i<fs->num_children; ++i){
            printfs(fs->child[i]);
        }
    }
}

Node* create_path(Node* fs, const char* path) {
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

int add_file(Node* fs, const char* path, const char* file_name, int overwrite) {
    if (overwrite == 0) 
        return insert_node(fs, path, FS_FILE, file_name);
    
    Node* dir = create_path(fs, path);
    return insert_child(dir, FS_FILE, file_name); 
}

int rand_in(int min_val, int max_val) {
    return (rand() % (max_val - min_val + 1) + min_val);
}

int* num_rand_in(int num_vals, int lower_limit, int upper_limit) {
    int *vals = malloc(num_vals  * sizeof(int));
    int i, j, r, repeat;
    for(i = 0; i<num_vals; ++i) {
        r = rand_in(lower_limit, upper_limit);
        repeat = 0;
        for(j = 0; j < i; ++j) {
            if (vals[j] == r) {
                repeat = 1;
                break;
            } 
        }
        if (!repeat) {
            vals[i] = r;
        } else {
            --i;
        }
    }
    return vals;
}

char* get_chunk_name(const char* path, int chunk_number) {
    char* chunk_name = malloc(512);
    sprintf(chunk_name, "%s%d", path, chunk_number);
    return chunk_name;
}

int* get_replica_addresses() {
    srand(time(0));
    int *rand_idxs = num_rand_in(NUM_REPLICAS, 0, count_d-1); 
    int *addresses = malloc(NUM_REPLICAS * sizeof(int));
    for(int i = 0; i < NUM_REPLICAS; ++i){
        addresses[i] = addr_d[rand_idxs[i]];
    }
    return addresses;
}

int add_chunk(Node* fs, const char* path) {
    Node* file = find_node(fs, path);
    FileMeta* meta = file->meta;
    
    if (meta->num_chunks >= (0.7*meta->buff_size)){
        meta->buff_size *= 2;
        meta->chunks = realloc(meta->chunks, meta->buff_size);
    }
    
    char* chunk_name = get_chunk_name(path, meta->num_chunks);
    int* addresses = get_replica_addresses();
    init_chunk(&(meta->chunks[meta->num_chunks++]), chunk_name, addresses);
}

int async_recv(Message* msg, int type) {
    if (msgrcv(msgid_m, msg, sizeof(Message), type, IPC_NOWAIT) != -1) 
	    return 1; 
    else return 0;
}

int main() {

    init_mesg_queue();
    
    // create_path(fs, "movies/batman/batman_begins");
    // find_node(fs, "superman");
    // printfs(fs);
    // char path[128];
    // strcpy(path,"/hello/file.txt/");
    // char** parts = parse_input(path, "/");
    // printf("%s\n", parts[1]);
    // exit(EXIT_SUCCESS);
    

    Node* fs = mknode(FS_DIR, "root");
    Message message;
    
    while(1){
        if (async_recv(&message, ADD_FILE)) {
            int status = add_file(fs,message.filepath, message.filename, 1);
            // Message msg; msg.mtype = OK; msgsnd(msgid_c, &msg, sizeof(msg), 0);
        }
        if (async_recv(&message, ADD_CHUNK)) add_chunk(fs, message.filepath);
    } 
	msgctl(msgid_m, IPC_RMID, NULL); 

    return 0;
}
