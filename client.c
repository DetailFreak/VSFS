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
    
    if (argc == 2) {
        int len = strlen(args[1]);
        if (args[1][len-1] == '\n')
            args[1][len-1] ='\0';

        if (args[1][0] != '/'){
            sprintf(req->filepath, "/%s", args[1]);
        } else {
            strcpy(req->filepath, args[1]);
        }

        req->msg_id = msgid_c;
    } 

    if (argc == 3) {

        if (args[1][0] == '-'){
            sprintf(req->text, "%c", args[1][1]);
        } else {
            req->text[0] = '\0';
        }

        int len = strlen(args[2]);
        if (args[2][len-1] == '\n')
            args[2][len-1] ='\0';

        if (args[2][0] != '/'){
            sprintf(req->filepath, "/%s", args[2]);
        } else {
            strcpy(req->filepath, args[2]);
        }

        req->msg_id = msgid_c;
    }

    if (sync_send(msgid_m, req) && sync_recv(msgid_c, res, ACK)){
        if (res->operation != OK)  {
            printf("M: ERROR: %s\n", res->text);
        }
    }         
}

void list_files(char **args, Message* req, Message* res) {
    int argc = count_args(args);
    
    req->mtype = LIST_FILES;
    req->msg_id = msgid_c;

    if(argc == 1) {
        strcpy(req->filepath,"/");
    }

    if (argc == 2) {
        int len = strlen(args[1]);
        if (args[1][len-1] == '\n')
            args[1][len-1] ='\0';

        if (args[1][0] != '/'){
            sprintf(req->filepath, "/%s", args[1]);
        } else {
            strcpy(req->filepath, args[1]);
        }
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
    req->mtype = DELETE_FILE;
    req->msg_id = msgid_c;

    if (argc == 2) {
        int len = strlen(args[1]);
        if (args[1][len-1] == '\n')
            args[1][len-1] ='\0';

        if (args[1][0] != '/'){
            sprintf(req->filepath, "/%s", args[1]);
        } else {
            strcpy(req->filepath, args[1]);
        }
    }

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
    req->mtype = ADD_CHUNK;
    
    int len = strlen(args[1]);
    if (args[1][len-1] == '\n')
        args[1][len-1] = '\0';
    
    if (args[1][0] != '/'){
        sprintf(req->filepath, "/%s", args[1]);
    } else {
        strcpy(req->filepath, args[1]);
    }
    
    req->msg_id = msgid_c;
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
    req->mtype = START_DSERVER;
    int len = strlen(args[1]);
    if (args[1][len-1] == '\n')
        args[1][len-1] = '\0';
    sprintf(req->text,"%s", args[1]);

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
