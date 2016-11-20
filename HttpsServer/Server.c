// ---- Base code ---- //
// Processes request from client
void get_request(int index_in_buffer){
	int i,j;
	int found_get;
	found_get=0;

	while (read_line(index_in_buffer,SIZE_BUF) > 0 )
	{
		if(!strncmp(buffer_of_requests->list_requests[index_in_buffer].buf_aux,GET_EXPR,strlen(GET_EXPR)))
		{
            // GET received, extract the requested page/script
			found_get=1;
			i=strlen(GET_EXPR);
			j=0;
			while( (buffer_of_requests->list_requests[index_in_buffer].buf_aux[i]!=' ') && (buffer_of_requests->list_requests[index_in_buffer].buf_aux[i]!='\0') )
				buffer_of_requests->list_requests[index_in_buffer].file_name[j++]=buffer_of_requests->list_requests[index_in_buffer].buf_aux[i++];
			buffer_of_requests->list_requests[index_in_buffer].file_name[j]='\0';
		}
	}
    // Currently only supports GET
	if(!found_get)
	{
		printf("Request from client without a GET\n");
		exit(1);
	}
    // If no particular page is requested then we consider htdocs/index.html
	if(!strlen(buffer_of_requests->list_requests[index_in_buffer].file_name))
		sprintf(buffer_of_requests->list_requests[index_in_buffer].file_name,"404.shtml");

    // get file type of the request
	buffer_of_requests->list_requests[index_in_buffer].type = get_file_type(index_in_buffer);
#if DEBUG
	printf("get_request: client requested the following page: %s\n",buffer_of_requests->list_requests[index_in_buffer].file_name);
#endif
	return;
}
// Reads a line (of at most 'n' bytes) from socket
int read_line(int index_in_buffer,int n){
	int n_read;
	int not_eol;
	int ret;
	char new_char;
	int socket = buffer_of_requests->list_requests[index_in_buffer].socket;
	n_read=0;
	not_eol=1;
	while (n_read<n && not_eol)
	{
		ret = read(socket,&new_char,sizeof(char));
		if (ret == -1)
		{
			printf("Error reading from socket (read_line)");
			return -1;
		}
		else if (ret == 0)
		{
			return 0;
		}
		else if (new_char=='\r')
		{
			not_eol = 0;
            // consumes next byte on buffer (LF)
			read(socket,&new_char,sizeof(char));
			continue;
		}
		else
		{
			buffer_of_requests->list_requests[index_in_buffer].buf_aux[n_read]=new_char;
			n_read++;
		}
	}
	buffer_of_requests->list_requests[index_in_buffer].buf_aux[n_read]='\0';
#if DEBUG
    //printf("read_line: new line read from client socket: %s\n",buffer_of_requests->list_requests[index_in_buffer].buf_aux);
#endif
	return n_read;
}
// Sends a 404 not found status message to client (page not found)
void not_found(int index_in_buffer){
	int socket=buffer_of_requests->list_requests[index_in_buffer].socket;
	char buf[SIZE_NAME];
	strcpy(buf,buffer_of_requests->list_requests[index_in_buffer].buf_aux);
	sprintf(buf,"HTTP/1.0 404 NOT FOUND\r\n");
	send(socket,buf, strlen(buf), 0);
	sprintf(buf,SERVER_STRING);
	send(socket,buf, strlen(buf), 0);
	sprintf(buf,"Content-Type: text/html\r\n");
	send(socket,buf, strlen(buf), 0);
	sprintf(buf,"\r\n");
	send(socket,buf, strlen(buf), 0);
	sprintf(buf,"<HTML><TITLE>Not Found</TITLE>\r\n");
	send(socket,buf, strlen(buf), 0);
	sprintf(buf,"<BODY><P>Resource unavailable or nonexistent.\r\n");
	send(socket,buf, strlen(buf), 0);
	sprintf(buf,"</BODY></HTML>\r\n");
	send(socket,buf, strlen(buf), 0);

	return;
}

// Creates, prepares and returns new socket
int fireup(int port){
	int new_sock;
	struct sockaddr_in name;

    // Creates socket
	if ((new_sock = socket(PF_INET, SOCK_STREAM, 0))==-1)
	{
		printf("Error creating socket\n");
		return -1;
	}

    // Binds new socket to listening port
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(new_sock, (struct sockaddr *)&name, sizeof(name)) < 0)
	{
		printf("Error binding to socket\n");
		kill(process_maker,SIGINT);
		 //clean shared memory
		shmdt(&shared_memory_id);
		shmctl(shared_memory_id,IPC_RMID,NULL);
		printf("Shared memory cleaned\n");
		return -1;
	}

    // Starts listening on socket
	if (listen(new_sock, 5) < 0)
	{
		printf("Error listening to socket\n");
		return -1;
	}

	return(new_sock);
}
// Send a 5000 internal server error (script not configured for execution)
void cannot_execute(int index_in_buffer){
	int socket=buffer_of_requests->list_requests[index_in_buffer].socket;
	char buf[SIZE_NAME];
	strcpy(buf,buffer_of_requests->list_requests[index_in_buffer].buf_aux);
	sprintf(buf,"HTTP/1.0 500 Internal Server Error\r\n");
	send(socket,buf, strlen(buf), 0);
	sprintf(buf,"Content-type: text/html\r\n");
	send(socket,buf, strlen(buf), 0);
	sprintf(buf,"\r\n");
	send(socket,buf, strlen(buf), 0);
	sprintf(buf,"<P>Error prohibited CGI execution.\r\n");
	send(socket,buf, strlen(buf), 0);

	return;
}
// Send message header (before html page) to client
void send_header(int index_in_buffer)
{
#if DEBUG
	printf("send_header: sending HTTP header to client\n");
#endif
	int socket = buffer_of_requests->list_requests[index_in_buffer].socket;
	sprintf(buffer_of_requests->list_requests[index_in_buffer].buf_aux,HEADER_1);
	send(socket,buffer_of_requests->list_requests[index_in_buffer].buf_aux,strlen(HEADER_1),0);
	sprintf(buffer_of_requests->list_requests[index_in_buffer].buf_aux,SERVER_STRING);
	send(socket,buffer_of_requests->list_requests[index_in_buffer].buf_aux,strlen(SERVER_STRING),0);
	sprintf(buffer_of_requests->list_requests[index_in_buffer].buf_aux,HEADER_2);
	send(socket,buffer_of_requests->list_requests[index_in_buffer].buf_aux,strlen(HEADER_2),0);
	return;
}