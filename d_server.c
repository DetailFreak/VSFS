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
	int chunk_num = 1;
	m->mtype = 3;

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
}


void copy_chunk(char *server_name, Message *req, Message *res){

	if (req->mtype == req->server_id) {
		printf("Chunk already written!\n");
		req->mtype = ACK;
		req->operation = OK;
		sprintf(req->text, "Chunk already written!\n");
		sync_send(msgid_m, req);
		return;
	}

	char srcfile[600], error[256];
	snprintf(srcfile,600,"%s%s", server_name, req->chunkname);
	
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
	res->operation = ADD_CHUNK;
	res->mtype = req->server_id;
	res->chunk_size = chunk_size;
	strcpy(res->chunkname, req->chunkname);
	
	if ((bytesRead = fread(res->text, chunk_size, 1 , file)) > 0){
		printf("----  %s -----, %lu chunk_size\n", res->chunkname, chunk_size);
		 if (sync_send(msgid_d, res) && sync_recv(msgid_d, req, ACK)){
			if (req->operation != OK)  {
				printf("D: ERROR: %s\n", req->text);
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