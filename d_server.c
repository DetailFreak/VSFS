#include "vsfs_config.h"

void add_chunk(char *server, Message *m){
	printf("Data %s \n", m->text);

	FILE *fp;
	char filename[256];
	snprintf(filename, 256, "%s/%s.txt", server, m->chunkname);

	fp = fopen(filename,"w");
	fwrite(m->text, m->chunk_size, 1, fp);
	fclose(fp);
}


void remove_chunk(char *server, Message *m){
	char filename[256];
	int chunk_num = 1;
	m->mtype = 3;

	// puts("inside remove chunk");

	puts(filename);
	snprintf(filename, 256, "%s/%s.txt", server, m->chunkname);

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


void send_chunk(long receiver_id, Message *m,  char *chunkname, char *server_name){
	FILE *file = NULL;
    size_t bytesRead = 0;
	char srcfile[128];

	m->operation = ADD_CHUNK;
	m->mtype = receiver_id;
	strcpy(m->chunkname, chunkname);
	sprintf(srcfile,"%s/%s.txt", server_name, m->chunkname);

	file = fopen(srcfile, "r");
	if(file != NULL){
		while ((bytesRead = fread(m->text, 1, 128, file)) > 0){
			printf("----  %s ----- \n", m->text);
			if(sync_send(msgid_d, m)){
				perror("add chunk send error");
			}
		}
		fclose(file);
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
	char servername[256];
	long server_id = atol(argv[1]);
	snprintf(servername, 256, "server_%ld", server_id);

	advertise_self(msgid_m, server_id, servername, &res);
    // msgctl(msgid_cd, IPC_RMID, NULL);
	// remove_chunk("server_3");
	if(server_id == 1)
		// send_chunk(2, "openflow_Group_select", 4, server_id, "server_1");
	do
	{
		if(async_recv(msgid_d, &req, server_id)) {
			printf("D:server_%d saw %s\n", server_id, req.text);
			if(req.operation == ADD_CHUNK){
				add_chunk(servername, &req); 
			}
			if(req.operation == RM_CHUNK){
				remove_chunk(servername, &req);
			}
			if(req.operation == STOP_DSERVER){
				res.mtype = ADV;
				res.operation = DEAD;
				res.server_id = server_id;
				strcpy(res.text, servername);
				sync_send(msgid_m, &res);
				exit(EXIT_SUCCESS); 
			}
		}
	} while (1);

	return 0; 
} 