#include "vsfs_config.h"

#define MAX_NAME_LEN 256
#define MAX_CHILDREN 512
#define FS_FILE 1
#define FS_DIR 0

#define INIT_BUFF_LEN = 100;

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
    printf("\tinitializing chunk \"%s\"\n", chunk_name);
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
    // printf("finding child \"%s\"\n", name);
    for(int i = 0; i < fs->num_children; ++i)
        if (strcmp(fs->child[i]->name, name) == 0){
            return fs->child[i];
        }
    return NULL;
}

int insert_child(Node* fs, int type, const char* name) {
    printf("\tinserting child \"%s\"\n", name);
    if (fs->num_children == MAX_CHILDREN){
        printf("Error insert_child: Node limit reached!\n");
        return -1;
    }
    if (find_child(fs, name)){
        printf("insert_child: Node with name \"%s\"already exists.\n", name);
        return -1;
    }
    if (fs->type == FS_FILE){
        printf("Error inserting: files cannot have any child nodes.\n");
        return -1;
    }
    fs->child[fs->num_children++] = mknode(type, name);
    return 0;
}

int find_child_idx(Node* fs, const char* name){
    // printf("finding child \"%s\"\n", name);

    for(int i = 0; i<fs->num_children; ++i)
        if (strcmp(fs->child[i]->name,name) == 0)
            return i;
    return -1;
}

int delete_child(Node *fs, const char* name, char* error) {
    printf("\tdeleting child \"%s\"\n", name);
    int i, j;
    if ((i = find_child_idx(fs, name)) == -1){
        sprintf(error,"Error deleting, Node not found!.\n");
        return -1;
    }

    if (fs->child[i]->type == FS_FILE){
        if (fs->child[i]->meta->num_chunks) 
            free(fs->child[i]->meta->chunks);
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
    // printf("finding path \"%s\"\n", pth);

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
    printf("\tinserting file \"%s\" at \"%s\"\n", name, pth);

    char *path = malloc(sizeof(pth));
    strcpy(path, pth);
    
    Node* parent;
    if ((parent  = find_node(fs, path)) == NULL) {
        printf("Error insert_node: invalid path\n");
        return -1;
    }
    if (insert_child(parent, type, name) == -1) {
        printf("Error insert_node: insert child failed\n");
        return -1;
    }
    return 0;
}

int delete_node(Node* fs, const char* pth, int type, const char* name, char * error) {
    printf("\tdeleting file \"%s\" at \"%s\"\n", name, pth);

    char *path = malloc(sizeof(pth));
    strcpy(path, pth);
    
    Node* parent;
    if ((parent  = find_node(fs, path)) == NULL) {
        printf("Error delete_node: invalid path\n");
        return -1;
    }
    if (delete_child(parent, name, error) == -1) {
        printf("Error delete_node: insert child failed\n");
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

int add_file(Node* fs, const char* path, const char* file_name, int overwrite, char *error) {

    if (overwrite == 0) {
        if (insert_node(fs, path, FS_FILE, file_name) == -1){
            sprintf(error, "Insert node failed!");
            return -1;
        } 
        return 0;
    }
    Node* dir = create_path(fs, path);
    if (insert_child(dir, FS_FILE, file_name) == -1) {
        printf("insert child failed! path:%s filename:%s\n", path, file_name);
        sprintf(error,"insert child failed!");
        return -1;
    }
    return 0; 
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
    int *rand_idxs = num_rand_in(NUM_REPLICAS, 0, count_d-1); 
    int *addresses = malloc(NUM_REPLICAS * sizeof(int));
    for(int i = 0; i < NUM_REPLICAS; ++i){
        addresses[i] = addr_d[rand_idxs[i]];
    }
    return addresses;
}

int add_chunk(Node* fs, const char* path, char* error) {
    printf("ADD_CHUNK: \"%s\"\n", path);

    Node* file = find_node(fs, path);
    if (!file) {
        sprintf(error, "File does not exist!");
        return -1;
    }
    FileMeta* meta = file->meta;
    if (meta->num_chunks >= (int)meta->buff_size - 10){
        meta->buff_size = (meta->buff_size > 0) ? (meta->buff_size * 2) : (10);
        meta->chunks = realloc(meta->chunks, (int)meta->buff_size*sizeof(Chunk));
    }
    
    char* chunk_name = get_chunk_name(path, meta->num_chunks);

    int* addresses = get_replica_addresses();

    init_chunk(&meta->chunks[meta->num_chunks++], chunk_name, addresses);
    return 0;
}

void add_file_req(Node* fs, Message *req, Message *res) {
    printf("ADD_FILE: %s\n", req->filepath);

    char error[128];
    char **parts = divide_path(req->filepath);
    
    res->mtype = ACK;
    if (add_file(fs, parts[0], parts[1], (req->text[0] == 'f'), error) != -1){
        printf("SUCESS\n");
        res->operation = OK;
    } else {
        printf("FAILURE\n");
        strcpy(res->text, error);
        res->operation = ERROR;
    }
    printfs(fs);
    sync_send(req->msg_id, res);
}

int duplicate_file(Node *fs, const char *src, const char* dst, int force, char *error) {
    printf("\tduplicating file from \"%s\" to \"%s\"\n", src, dst);
    
    Node *fsrc = find_node(fs, src), *fdst = find_node(fs, dst);
    
    if (!fsrc) {
        printf("source file \"%s\" does not exist!\n",src);
        return -1;
    }
    if (fdst) {
        printf("destination file \"%s\" already exists!",dst);
        return -1;
    }

    char ** dst_parts = divide_path(dst);
    if (add_file(fs, dst_parts[0], dst_parts[1], force, error) == -1) {
        printf("add file failed\n");
        return -1;
    } fdst = find_node(fs, dst);

    int num_chunks = fsrc->meta->num_chunks;
    fdst->meta->num_chunks = num_chunks;
    fdst->meta->chunks = malloc(sizeof(Chunk) * fsrc->meta->buff_size);
    fdst->meta->buff_size = fsrc->meta->buff_size;

    for(int i = 0; i<num_chunks; ++i) {
        init_chunk(fdst->meta->chunks + i, fsrc->meta->chunks[i].name, fsrc->meta->chunks[i].addr);
    }

    return 0;
}

int move_file(Node *fs, const char *src, const char* dst, int force, char *error) {
    if (duplicate_file(fs, src, dst, force, error) == -1) {
        printf("Couldn't duplicate file\n");
        return -1;
    }
    char ** parts = divide_path(src);
    if (delete_node(fs, parts[0], FS_FILE, parts[1], error) == -1) {
        printf("Error in delete_node\n");
        return -1;
    }
    return 0;
}

int copy_file(Node* fs, const char *src, const char* dst, int force, char *error) {
    printf("\tcopying file from \"%s\" to \"%s\" %d\n",  src, dst, force);
    
    Node *fsrc = find_node(fs, src), *fdst = find_node(fs, dst);
    
    if (!fsrc) {
        printf("source file \"%s\" does not exist!\n",src);
        return -1;
    }
    if (fdst) {
        printf("destination file \"%s\" already exists!",dst);
        return -1;
    }

    char ** dst_parts = divide_path(dst);
    if (add_file(fs, dst_parts[0], dst_parts[1], force, error) == -1) {
        printf("add file failed\n");
        return -1;
    }

    int num_chunks = fsrc->meta->num_chunks;
    for(int i = 0; i<num_chunks; ++i) {
        add_chunk(fs,dst, error);
    }

    return 0;
}

void print_chunks(Node* fs, const char * path) {
    Node *file = find_node(fs, path);
    
    if (file) {
        
        printf("Chunk locations for \"%s\":\n", file->name);
        FileMeta *meta = file->meta;
       
        for(int i = 0; i<meta->num_chunks; ++i) {
            printf("\tChunk %d -> ", i+1);
            for(int j = 0; j < NUM_REPLICAS; ++j) {
                printf("%d ", meta->chunks[i].addr[j]);
            } printf("\n");
        }
    } else {
        printf("File \"%s\" does not exist!\n", path);
    }
}

void copy_file_req(Node* fs, Message *req, Message *res)  {
    printf("COPY: %s %s\n", req->filepath, req->chunkname);
    
    char error[128];
    if (copy_file(fs, req->filepath, req->chunkname, (req->text[0] == 'f'), error) != -1) {
        printf("SUCCESS\n");
        res->operation = OK;
    } else {
        strcpy(res->text, error);
        printf("FAILURE\n");
        res->operation = ERROR;
    }

    res->mtype = ACK;
    print_chunks(fs, req->chunkname);
    sync_send(req->msg_id, res);
}

void add_chunk_req(Node* fs, Message *req, Message *res) {

    char error[128];
    if (add_chunk(fs, req->filepath, error) != -1){
        printf("SUCESS\n");
        Node *file = find_node(fs, req->filepath);
        Chunk *chunk = &file->meta->chunks[file->meta->num_chunks-1];
        strcpy(res->chunkname, chunk->name);
        memcpy(res->addr_d, chunk->addr, NUM_REPLICAS*sizeof(int));
        sprintf(res->text, "\tChunk [%d] -> [", file->meta->num_chunks);
        for(int i=0; i<NUM_REPLICAS; ++i){
            sprintf(res->text + strlen(res->text),"%d, ", chunk->addr[i]);
        } sprintf(res->text + strlen(res->text),"]");

        res->operation = OK;
    } else {
        printf("FAILURE\n");
        res->operation = ERROR;
        strcpy(res->text, error);
    }
    res->mtype = ACK;
    print_chunks(fs, req->filepath);
    sync_send(req->msg_id, res);
}

void delete_file_req(Node* fs, Message *req, Message *res) {
    printf("DELETE_FILE: %s\n", req->filepath);

    char error[128];
    char ** parts = divide_path(req->filepath);
    if (delete_node(fs, parts[0], FS_FILE, parts[1], error) != -1){
        printf("SUCESS\n");
        res->operation = OK;
    } else {
        printf("FAILURE\n");
        res->operation = ERROR;
        strcpy(res->text, error);
    }
    res->mtype = ACK;
    print_chunks(fs, req->filepath);
    sync_send(req->msg_id, res);
    free(parts);
} 

void move_file_req(Node* fs, Message *req, Message *res) {
    printf("MOVE: %s %s\n", req->filepath, req->chunkname);

     char error[128];
    if (move_file(fs, req->filepath, req->chunkname, (req->text[0] == 'f'), error) != -1) {
        printf("SUCCESS\n");
        res->operation = OK;
    } else {
        strcpy(res->text, error);
        printf("FAILURE\n");
        res->operation = ERROR;
    }

    res->mtype = ACK;
    print_chunks(fs, req->chunkname);
    sync_send(req->msg_id, res);
    
}


void list_files(Node*fs, Message *req, Message * res) {
    printf("LIST_FILES: %s\n", req->filepath);
    if (strcmp(req->filepath, "") == 0 || strcmp(req->filepath, "/") == 0)
        fs = fs;
    else fs = find_node(fs, req->filepath);

    if (fs && fs->type == FS_DIR) {
        res->text[0] = '\0';
        for(int i = 0; i<fs->num_children; ++i)
            sprintf(res->text + strlen(res->text),"%s (%s)\t", fs->child[i]->name, type2str(fs->child[i]->type));
        res->operation = OK;
    } else {
        res->operation = ERROR;
        if (!fs)
            strcpy(res->text, "Path not found");
        else if (fs->type == FS_FILE)
             strcpy(res->text, "Cannot list: This is a file, not a directory");
    }
    res->mtype = ACK;
    sync_send(req->msg_id, res);
}

void start_dserver(Node* fs, Message *req, Message *res){
    printf("START_DSERVER: %s\n", req->text);
    int num_servers = atoi(req->text);
    int max_server_id = 0;
    for (size_t i = 0; i < count_d; i++)
    {
        max_server_id = (addr_d[i] > max_server_id) ? (addr_d[i]) : (max_server_id);
    }
    
    pid_t pid;
    for(int i=1; i<=num_servers; ++i)
    {
        int new_server_id = max_server_id + i;

        pid = fork();
        if (pid == 0){
            char mkdir_command[64], idx_str[64];
            sprintf(mkdir_command, "mkdir server_%d", new_server_id);
            system(mkdir_command);
            sprintf(idx_str,"%d", new_server_id);
            execlp("./d_server.out", "d_server.out", idx_str, NULL);
            perror("execlp");
            exit(EXIT_SUCCESS);
        }
    }
    res->mtype = ACK;
    res->operation = OK;
    sync_send(msgid_c, res);
}


void add_dserver(int server_id) {
    addr_d[count_d++] = server_id;
}

void remove_dserver(int server_id) {
     for(int i = 0; i<count_d; ++i) {
        if (addr_d[i] == server_id){
            addr_d[i] = addr_d[count_d-1];
            count_d--;
            break;
        }
    }
}

void stop_dserver(Node* fs, Message *req, Message *res) {
    printf("STOP_DSERVER: \n");
    res->operation = STOP_DSERVER;
    int killed = 0;
    int* killed_server_ids = malloc(sizeof(count_d));
    for(int i = 0; i<count_d; ++i) {
        printf("message sent to addr_d[%d]=%d\n", i, addr_d[i]);
        res -> mtype = addr_d[i];
        strcpy(res -> text, "STOP_DSERVER");
        if (sync_send(msgid_d, res) && sync_recv(msgid_d, req, ADV)){
            killed_server_ids[i] = res->server_id;
            killed++;
        }
    }
    for(int i = 0; i<killed; ++i) {
        remove_dserver(killed_server_ids[i]);
    }
    free(killed_server_ids);

    res->mtype = ACK;
    res->operation = OK;
    sync_send(msgid_c, res);
}

void print_dservers() {
    printf("DSERVERS UP: ");
    for (size_t i = 0; i < count_d; i++)
    {
        printf("%d, ",addr_d[i]);
    } printf("\n");
}

void handle_advertisements(Message *req) {
    if(req->operation == OK){
        // printf("D:%s , serverid:%d is up\n", req->text, req->server_id);
        add_dserver(req->server_id);
    } 
    else if (req->operation == DEAD) {
        // printf("D:%s is down\n", req->text);
        remove_dserver(req->server_id);
    }
    }

int main() {
    srand(time(0));
    init_mesg_queue();
    count_d = 0;
    
    // create_path(fs, "movies/batman/batman_begins");
    // find_node(fs, "superman");
    // printfs(fs);
    // char path[128];
    // strcpy(path,"/hello/file.txt/");
    // char** parts = parse_input(path, "/");
    // printf("%s\n", parts[1]);
    // exit(EXIT_SUCCESS);
    

    Node* fs = mknode(FS_DIR, "root");
    Message req;
    Message res;
    
    while(1){
      
        if (async_recv(msgid_m, &req, ADD_FILE)) {
            add_file_req(fs, &req, &res);
        }
        if (async_recv(msgid_m, &req, DELETE_FILE)) {
            delete_file_req(fs, &req, &res);
        }
        if (async_recv(msgid_m, &req, ADD_CHUNK)) {
            add_chunk_req(fs, &req, &res);
        }
        if (async_recv(msgid_m, &req, COPY)) {
            copy_file_req(fs, &req, &res);
        }
        if (async_recv(msgid_m, &req, MOVE)) {
            move_file_req(fs, &req, &res);
        }
        if (async_recv(msgid_m, &req, LIST_FILES)) {
            list_files(fs, &req, &res);
        }
        if (async_recv(msgid_m, &req, START_DSERVER)) {
            start_dserver(fs, &req, &res);
        }
        if (async_recv(msgid_m, &req, STOP_DSERVER)) {
            stop_dserver(fs, &req, &res);
        }

        if (async_recv(msgid_m, &req, ADV)) {
            handle_advertisements(&req);
            print_dservers();
        }
        //     printf("Got ADD_CHUNK Message: %s", req.text);

        //     // add_chunk_req(fs, &req, &res);
            
        // sync_send(req.msg_id, &res);
        // }
    } 
	msgctl(msgid_m, IPC_RMID, NULL); 

    return 0;
}
