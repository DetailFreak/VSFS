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

    if (argc == 4) {
        char *flag = args[1];
        src = args[2];
        dst = args[3];
        printf("%s\n", flag);
        if (flag[0] == '-') sprintf(req->text, "%c", flag[1]);
            else req->text[0] = '\0';
    }

    remove_newline(src);
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

    if (argc == 4) {
        char *flag = args[1];
        src = args[2];
        dst = args[3];
        if (flag[0] == '-') sprintf(req->text, "%c", flag[1]);
            else req->text[0] = '\0';
    }
  
    remove_newline(src);
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
        
        else if (str_equals(args[0], "startd")){
            start_data_server(args, &req, &res);
        }

        else if (str_equals(args[0], "stopd")){
            stop_data_server(args, &req, &res);
        }
	}
	return 0; 
} 
