#include "vsfs_config.h"

char* get_input(){
    char *buf;
    size_t size = 1024;
    buf = (char*) malloc(size);
    printf("VSFS> ");
    fflush(stdout);
    if (getline(&buf, &size, stdin) == -1){
            exit(EXIT_FAILURE);
    }
    return buf;
}

/*
    Request response functions
*/

void add_file(char **args, Message* req, Message* res){
	int argc = count_args(args);
    if (argc < 2 || argc > 3) {
        printf("Usage: af [optional -f] <path to file> \n");
        return;
    }

    req->mtype = ADD_FILE;
    req->msg_id = msgid_c;
    
    if (argc == 2) {
        char * filepath = args[1];
        remove_newline(filepath);
        copy_path(req->filepath, filepath);
    } 

    req->text[0] = '\0';

    if (argc == 3) {
        char * flag = args[1];
        char * filepath = args[2];
        remove_newline(filepath);
        copy_path(req->filepath, filepath);
        if (flag[0] == '-') sprintf(req->text, "%c", flag[1]);
            else req->text[0] = '\0';
    }

    if (sync_send(msgid_m, req) && sync_recv(msgid_c, res, ACK)){
        if (res->operation != OK)  {
            printf("M: ERROR: %s\n", res->text);
        }
    }         
}

void list_files(char **args, Message* req, Message* res) {
    int argc = count_args(args);
    if (argc > 2) {
        printf("Usage: ls  [optional path] \n");
        return;
    }  
    req->mtype = LIST_FILES;
    req->msg_id = msgid_c;

    if(argc == 1) {
        copy_path(req->filepath,"/");
    }

    if (argc == 2) {
        char* path = args[1];
        remove_newline(path);
        copy_path(req->filepath, path);
    }

     if (sync_send(msgid_m, req) && sync_recv(msgid_c, res, ACK)){
        if (res->operation != OK)  {
            printf("M: ERROR: %s\n", res->text);
        } else {
            printf("%s\n", res->text);
        }
    }
}

void delete_file(char **args, Message* req, Message* res) {
    int argc = count_args(args);
    if (argc != 2) {
        printf("Usage: rm <path to file> \n");
        return;
    }    

    char* path = args[1];
    remove_newline(path);

    req->mtype = DELETE_FILE;
    req->msg_id = msgid_c;
    copy_path(req->filepath, path);

    if (sync_send(msgid_m, req) && sync_recv(msgid_c, res, ACK)){
        if (res->operation != OK)  {
            printf("M: ERROR: %s\n", res->text);
        } 
    }
}

void move_file(char **args, Message* req, Message* res){
    int argc = count_args(args);
    if (argc < 3 || argc > 4) {
        printf("Usage: mv [-f] <path to sourcefile> <path to destination>\n");
        return;
    }

    char *src, *dst;
    if (argc == 3){
        src = args[1];
        dst = args[2];
    }

    req->text[0] = '\0';

    if (argc == 4) {
        char *flag = args[1];
        src = args[2];
        dst = args[3];
        printf("%s\n", flag);
        if (flag[0] == '-') sprintf(req->text, "%c", flag[1]);
            else req->text[0] = '\0';
    }

    remove_newline(dst);

    req->mtype = MOVE;
    req->msg_id = msgid_c;
    copy_path(req->filepath, src);
    copy_path(req->chunkname, dst);

    if (sync_send(msgid_m, req) && sync_recv(msgid_c, res, ACK)){
        if (res->operation != OK)  {
            printf("M: ERROR: %s\n", res->text);
        }
    }    
}

void copy_file(char **args, Message* req, Message* res){
    int argc = count_args(args);
    if (argc < 3 || argc > 4) {
        printf("Usage: cp [-f] <path to sourcefile> <path to destination>\n");
        return;
    }
    
    char *src, *dst;
    if (argc == 3){
        src = args[1];
        dst = args[2];
    }

    req->text[0] = '\0';

    if (argc == 4) {
        char *flag = args[1];
        src = args[2];
        dst = args[3];
        if (flag[0] == '-') sprintf(req->text, "%c", flag[1]);
            else req->text[0] = '\0';
    }
  
    remove_newline(dst);

    req->mtype = COPY;
    req->msg_id = msgid_c;
    copy_path(req->filepath, src);
    copy_path(req->chunkname, dst);

    if (sync_send(msgid_m, req) && sync_recv(msgid_c, res, ACK)){
        if (res->operation != OK)  {
            printf("M: ERROR: %s\n", res->text);
        }
    }    
}

void add_chunks(char **args, Message* req, Message* res){
    int argc = count_args(args);
    if (argc != 2) {
        printf("Usage: ac <path to file>\n");
        return;
    }

    char* path = args[1];
    remove_newline(path);
    
    req->mtype = ADD_CHUNK;
    req->msg_id = msgid_c;
    copy_path(req->filepath, path);
    
    if (sync_send(msgid_m, req) && sync_recv(msgid_c, res, ACK)){
        if (res->operation != OK)  {
            printf("M: ERROR: %s\n", res->text);
        } else {
            printf("%s\n", res->text);
        }
    }    
}

void exec_file(char **args, Message* req, Message *res) {
    int len = count_args(args);
    if (len != 3) {
        printf("Usage: exec <command> <filepath>\n");
        return;
    }
    char *command = args[1];
    char *filepath = args[2];
    remove_newline(filepath);
    
    req->msg_id = msgid_c;
    int num_chunks = 1;
    for(int i = 0; i < num_chunks; ++i) {
        req->mtype = CHUNK_INFO;
        sprintf(req->text, "%d", i);
        copy_path(req->filepath, filepath);
        if (sync_send(msgid_m, req) && sync_recv(msgid_c, res, ACK) && res->operation != OK)  {
            printf("M: ERROR: %s\n", res->text);
            break;
        }
        if (i == 0) num_chunks = res->chunk_size;
        req->mtype = res->addr_d[0];
        req->operation = EXEC_CHUNK;
        strcpy(req->text, command);
        strcpy(req->chunkname, res->chunkname);
        // printf("Chunk Info [%d]: name = %s, addr_d = [%d, %d, %d] \n", i+1, res->chunkname,  res->addr_d[0], res->addr_d[1], res->addr_d[2]);
        
        if (sync_send(msgid_d, req) && sync_recv(msgid_c, res, ACK) && res->operation == OK) {
            printf("%s",res->text);
        }
        memset(res->text, '\0', MAX_SIZE);
    }
    printf("\n");
}

void start_data_server(char **args, Message* req, Message *res) {
    int argc = count_args(args);
    if (argc != 2) {
        printf("Usage: startd <numservers>\n");
        return;
    }
    
    char *num_servers = args[1];
    remove_newline(num_servers);

    req->mtype = START_DSERVER;
    req->msg_id = msgid_c;
    strcpy(req->text, num_servers);

    if (sync_send(msgid_m, req) && sync_recv(msgid_c, res, ACK)){
        if (res->operation != OK) {
            printf("M: ERROR: %s\n", res->text);
        }
    }
}

void stop_data_server(char **args, Message* req, Message *res) {

    req->mtype = STOP_DSERVER;
    if (sync_send(msgid_m, req) && sync_recv(msgid_c, res, ACK)){
        if (res->operation != OK) {
            printf("M: ERROR: %s\n", res->text);
        }
    }
}

char* get_chunk_name(const char* path, int chunk_number) {

    char* chunk_name = malloc(512);
    sprintf(chunk_name, "%s%d", path, chunk_number);
    return chunk_name;
}


void send_file(char **args, Message* req, Message* res){
    int argc = count_args(args);
    if(argc != 4){
        puts("Usage: sf <path-to-file-in-vsfs> <path-to-file-in-local> <chunk-size>");
        return;
    }
    char * vsfs_file_path = args[1];
    char * local_file_path = args[2];
    char * chunk_size_str = args[3];
    remove_newline(chunk_size_str);

    req->msg_id = msgid_c;
    req->chunk_size = atoi(chunk_size_str);
    copy_path(req->filepath, vsfs_file_path);

    FILE* local_file = NULL;
    if ((local_file = fopen(local_file_path, "r")) == NULL) {
        printf("Error: Source path \"%s\" does not exist!\n", local_file_path);
        return;
    }

    printf("progress: =>");

    int chunks = 0;
    size_t bytes_read = 0;
    while((bytes_read = fread(req->text, 1 , req->chunk_size, local_file)) > 0){
        //add chunk request
        req->mtype = ADD_CHUNK;
        if (sync_send(msgid_m, req) && sync_recv(msgid_c, res, ACK)){
            if (res->operation != OK)  {
                printf("M: ERROR: %s\n", res->text);
            } else {
                // printf("%s\n", res->text);
            }
        }

        int addr[NUM_REPLICAS];
        char chunk_name[MAX_SIZE];
        memcpy(addr, res->addr_d, sizeof(int) * NUM_REPLICAS);
        strcpy(chunk_name, res->chunkname);

        for(int i=0; i<NUM_REPLICAS; i++){
            req->mtype = addr[i];
            req->operation = ADD_CHUNK;
            req->msg_id = msgid_c;
            strcpy(req->chunkname, chunk_name);
            // printf("----- %d ---- %d ------ %s \n", req->msg_id, req->chunk_size, req->chunkname);

            if (sync_send(msgid_d, req) && sync_recv(msgid_c, res, ACK)){
                if (res->operation != OK)  {
                    printf("D: ERROR: %s\n", res->text);
                } else {
                    printf("\b=>");
                    fflush(stdout);
                }
            }   
        }

        ++chunks;
    }
    printf("| 100%%, %d bytes sent in %d chunks .\n", chunks * req->chunk_size, chunks);
}

int str_equals(char*a, const char *b) {
    return (strcmp(a, b) == 0);
}

int main() 
{	init_mesg_queue();

    char *input;
    char **args;

	Message req;
    Message res;

	while(1){

        if(*(input = get_input()) == '\n') continue;
        
        if (str_equals(input, "exit\n")){
            exit(EXIT_SUCCESS);
        }
        
        char **args = parse_input(input, " ");
        
        if (str_equals(args[0], "af")){
            add_file(args, &req, &res);
        } 
        
        else if (str_equals(args[0], "ac")){
            add_chunks(args, &req, &res);
        }

        else if(str_equals(args[0], "sf")){
            send_file(args, &req, &res);
        }

        else if (str_equals(args[0], "ls")){
            list_files(args, &req, &res);
        }

        else if (str_equals(args[0], "mv")){
            move_file(args, &req, &res);
        }
        
        else if (str_equals(args[0], "cp")){
            copy_file(args, &req, &res);
        }

        else if (str_equals(args[0], "rm")){
            delete_file(args, &req, &res);
        }
        
        else if (str_equals(args[0], "exec")){
            exec_file(args, &req, &res);
        }
        
        else if (str_equals(args[0], "d")){
            start_data_server(args, &req, &res);
        }

        else if (str_equals(args[0], "stopd")){
            stop_data_server(args, &req, &res);
        }
	}
	return 0; 
} 
