#include "vsfs_config.h"
Message req;
Message res;

void add_chunk(char *server, Message *m){
	// printf("Chunk %s \n", m->text);
	FILE *fp;
	char filename[600];
	snprintf(filename, 600, "%s/%s", server, m->chunkname);

	fp = fopen(filename,"w");
	fwrite(m->text, m->chunk_size, 1, fp);
	fclose(fp);

	m->mtype = ACK;
    m->operation = OK;
    sync_send(m->msg_id, m);
}


void remove_chunk(char *server, Message *m){
	char filename[600];

	// puts("inside remove chunk");
	snprintf(filename, 600, "%s/%s", server, m->chunkname);
	// puts(filename);

	while( access(filename, F_OK ) != -1 ) {
    	// file exists
		if(remove(filename) == 0){
			printf("--------- chunk %s removed \n", m->chunkname);	
		}else{
			printf("--------- Unable to delete chunk %s \n", m->chunkname);	
		}
		sprintf(filename, "%s/%s.txt", server, m->chunkname); 
	} 
	m->mtype = ACK;
	m->operation = OK;
	sync_send(msgid_m, m);

}


void copy_chunk(char *server_name, Message *req, Message *res){

	if (req->mtype == req->server_id) {
		char srcfile[600], dstfile[600];
		snprintf(srcfile,600,"%s/%s", server_name, req->chunkname);
		snprintf(dstfile,600,"%s/%s", server_name, req->text);
		
		char command[1280];
		sprintf(command, "cp %s %s", srcfile, dstfile);
		
		system(command);

		req->mtype = ACK;
		req->operation = OK;
		sync_send(msgid_m, req);
		return;
	}

	char srcfile[600], error[256];
	snprintf(srcfile,600,"%s/%s", server_name, req->chunkname);
	
	FILE *file = NULL;
    size_t bytesRead = 0;

	file = fopen(srcfile, "r");
	if (!file) {
		printf("Source chunk \"%s\" does not exist!\n", req->chunkname);
		req->mtype = ACK;
		req->operation = ERROR;
		sprintf(req->text, "Source chunk \"%s\" does not exist!\n", res->chunkname);
		sync_send(msgid_m, req);
		return;
	}
	
	fseek (file, 0, SEEK_END);
  	size_t chunk_size = ftell (file);
  	fseek (file, 0, SEEK_SET);

	res->msg_id = msgid_d;
	sprintf(res->text,"%ld", req->mtype);
	res->operation = ADD_CHUNK;
	res->mtype = req->server_id;
	res->chunk_size = chunk_size;
	strcpy(res->chunkname, req->text);
	
	if ((bytesRead = fread(res->text, 1 , chunk_size , file)) > 0){
		printf("----  %s -----, %lu chunk_size\n", res->chunkname, chunk_size);
		 if (sync_send(msgid_d, res) && sync_recv(msgid_d, req, ACK)){
			if (req->operation != OK)  {
				printf("D: ERROR: %s\n", req->text);
				req->mtype = ACK;
				req->operation = ERROR;
				strcpy(req->text, "Chunk not written");
				sync_send(msgid_m, req);
				printf(" Chunk \"%s\" not written\n", res->chunkname);
			} else {
				req->mtype = ACK;
				req->operation = OK;
				sync_send(msgid_m, req);
				printf(" Chunk \"%s\" succesfully written\n", res->chunkname);
			}
        } 
	}

	fclose(file);
}

void exec_chunk(char *server_name, Message *req, Message *res) {
/*
	TODO: make a pipe and execute funtion in a child process.
	read output using "read", send result back to callee.
*/
	char path[512];
	sprintf(path, "%s%s", server_name, req->chunkname);
	printf("exec command: %s, path: %s\n",req->text, path); 

	int pipe_fds[2];
	if (pipe(pipe_fds) == -1) {
		perror("pipe");
	} else {
		int pid = fork();
		if (pid == 0) {
		/*
			Child process
		*/
			if (close(pipe_fds[0]) == -1) {
				perror("pipe close");
			} else {
				if (dup2(pipe_fds[1], 1) == 0) {
					perror("dup2");
				} else {
					execlp(req->text, req->text, path, (char*)NULL);
					perror("exec");
					req->mtype = ACK;
					req->operation =  OK;
					sync_send(msgid_c, req);
				}
			}
		} else if (pid > 0) {
		/*
			parent process
		*/	if (close(pipe_fds[1]) == -1) {
				perror("pipe close");
			}
			ssize_t bytes_read;
			if ((bytes_read = read(pipe_fds[0], req->text, 1000)) == -1){
				perror("read");
				exit(EXIT_FAILURE);
			}
			req->text[bytes_read] = '\0';

			close(pipe_fds[0]);

			req->mtype = ACK;
			req->operation = OK;
			sync_send(req->msg_id, req);

		} else {
		/*
			fork error
		*/
			perror("fork");
			exit(EXIT_FAILURE);
		}
	}
}

void advertise_self(int msgid, int serverid, char* servername, Message* msg) {
	msg->mtype = ADV;
	msg->operation = OK;
	msg->server_id = serverid;
	strcpy(msg->text, servername);
	sync_send(msgid_m, msg);
}


int main(int argc, char** argv)
{
	init_mesg_queue();

	Message req, res;
	char servername[512];
	long server_id = atol(argv[1]);
	snprintf(servername, 256, "server_%ld", server_id);

	advertise_self(msgid_m, server_id, servername, &res);
    // msgctl(msgid_c, IPC_RMID, NULL);
	// remove_chunk("server_3");
	// if(server_id == 1)
	// 	send_chunk(2, "openflow_Group_select", 4, server_id, "server_1");
	do
	{
		if(async_recv(msgid_d, &req, server_id)) {
			// printf("D:server_%d saw %s\n", server_id, req.text);
			if(req.operation == ADD_CHUNK){
				add_chunk(servername, &req); 
			}
			if(req.operation == RM_CHUNK){
				remove_chunk(servername, &req);
			}
			if(req.operation == COPY_CHUNK){
				copy_chunk(servername, &req, &res);
			}
			if(req.operation == EXEC_CHUNK){
				exec_chunk(servername, &req, &res);
			}
			/*
				TODO: Execute functions
			*/
			if(req.operation == STOP_DSERVER){
				res.mtype = ADV;
				res.operation = DEAD;
				res.server_id = server_id;
				strcpy(res.text, servername);
				msgctl(msgid_d, IPC_RMID, NULL);
				sync_send(msgid_m, &res);
				exit(EXIT_SUCCESS); 
			}
		}
	} while (1);

	return 0; 
} 