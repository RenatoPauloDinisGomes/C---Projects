/*
 * -- simplehttpd.c --
 * A (very) simple HTTP server
 *       gcc simplehttpd.c -lpthread -D_REENTRANT -Wall -o simplehttpd
 * Sistemas Operativos 2016/2017
 */
/*

	- Arranque do servidor e aplicação das configurações existentes no ficheiro “config.txt” -> Done	
	- Criação de todos os processo filho necessários -> Done	
	- Criação da memória partilhada -> Done
	- Criação da pool de threads -> Done

- Criação do named pipe -> NOT DONE
- Leitura correcta dos comandos recebidos pelo named pipe -> NOT DONE
- Aplicação das configurações recebidas pelo named pipe -> NOT DONE
- Suporte do scheduler e política de escalonamento -> NOT DONE
- Suporte de concorrência no tratamento de pedidos -> NOT DONE

	- Leitura correcta dos pedidos feitos por HTTP e a sua colocação no buffer de pedidos -> Done need to see again 
	- Resposta a pedidos de páginas estáticas -> Done isto já não vinha feito?

- Resposta a ficheiros comprimidos (extensão .gz) -> NOT DONE
- Sincronização com mecanismos adequados (semáforos, mutexes ou variáveis de condição) -> NOT DONE
- Prevenir interrupções indesejada por sinais e fornecer a resposta adequada aos vários sinais -> NOT DONE
- Utilização da memória partilhada -> NOT DONE
- Ler comandos do utilizador -> NOT DONE
- Enviar comandos através do named pipe -> NOT DONE
- Ler informação da memória partilhada -> NOT DONE
- Escrever a informação estatística no ficheiro mapeado em memória (“server.log”) -> NOT DONE
- Enviar informação agregada para o écran -> NOT DONE
- Fazer reset a estatísticas agregadas -> NOT DONE

	- Deteção e tratamento de erros -> Done 75 % 
	- Terminação dos processos filho quando o processo principal termina. Libertação de recursos e limpeza ao terminar a aplicação. -> Done

*/
#include "headers.h"

int main(int argc, char ** argv){
	int process_maker;
		//Load Configs
	Load_Configs();
		//printf Configs
	print_Configs();
		//creat semaphores for threads
	create_semaphores();
		//create shared variable to save Statistics
	create_shared_variable();
		//create buffer_request
	buffer_of_requests = (struct buffer*)malloc(sizeof(struct buffer));
	buffer_of_requests->list_requests = (struct request*)malloc(MAX_REQUESTS*sizeof(struct request));
	buffer_of_requests->requests_attended=0;
	buffer_of_requests->total_requests=0;
		//create Processes
	process_maker=fork();

	if(process_maker==0){
		//process child for server Statistics
		server_stats();
	}else{
		//process father
		//Create Thread pool
		create_thread_pool();
		// to start http server
		start_server();
	}
}
void create_shared_variable(){
	if((shared_memory_id=shmget(IPC_PRIVATE,sizeof(struct stats),IPC_CREAT | 0777))<0){
		perror("ERROR CREATING SHARED MEMORY FOR STATISTICS");
		exit(1);
	}
	if((Statistics = (Stats)shmat(shared_memory_id,NULL,0))==(Stats)-1){
		perror("ERROR ATTACHING MEMORY");
		exit(1);
	}
}
void create_semaphores(){
	sem_init(&sem_threads,0,1);
	sem_wait(&sem_threads);
		//sem_getvalue(&sem_threads, &value);
		//printf("valor %d\n",value);
}
void * threads_handler(){

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

			//function to get id do proximo request no buffer -> to be done
	sem_wait(&sem_threads);
	pthread_mutex_lock(&simple_mutex);
	int id = id_current_request;
	id_current_request++;
	attending_request++;


	printf("THREAD ATTENDING REQUEST %i\n",id_current_request);
		// Identify new client
	identify(new_conn);
		// Process request
	get_request(new_conn);
		// Verify if request is for a page or script
	if(!strncmp(req_buf,CGI_EXPR,strlen(CGI_EXPR)))
		execute_script(new_conn);
	else
			// Search file with html page and send to client
		send_page(new_conn,id);
		// Terminate connection with client
	close(new_conn);

	
	buffer_of_requests->list_requests[id].attended=1;
	buffer_of_requests->requests_attended++;
	current_requests--;
	attending_request--;
	pthread_mutex_unlock(&simple_mutex);
		//sem_post(&sem_threads);
	sem_post(&sem_threads);
	pthread_exit(NULL);
	return 0;
}
/*void * threads_handler(){

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
	while(!changing_policy){
		pthread_mutex_lock(&simple_mutex);

		while(current_requests > Configurations->thread_pool_size || current_requests == 0 || !(current_requests - attending_request > 0)){
			pthread_cond_wait(&threads_cond_var,&simple_mutex);
		}
			//function to get id do proximo request no buffer -> to be done
		int id = id_current_request;
		id_current_request++;
		attending_request++;
		pthread_mutex_unlock(&simple_mutex);

		sleep(5);

		printf("THREAD ATTENDING REQUEST %i\n",id_current_request);

		// Identify new client
		identify(new_conn,id);

		// Process request
		get_request(new_conn,id);

		// Verify if request is for a page or script
		if(!strncmp(buffer_of_requests->list_requests[id].file_name,CGI_EXPR,strlen(CGI_EXPR)))
			execute_script(new_conn);
		else
			// Search file with html page and send to client
			send_page(new_conn,id);

		// Terminate connection with client
		close(new_conn);
		
		pthread_mutex_lock(&simple_mutex);
		buffer_of_requests->list_requests[id].attended=1;
		buffer_of_requests->requests_attended++;
		current_requests--;
		attending_request--;
		pthread_mutex_unlock(&simple_mutex);
		//sem_post(&sem_threads);

		//signal que acabou um request caso exista algum em espera
		pthread_cond_signal(&threads_cond_var);

	}
	pthread_exit(NULL);
	return 0;
}*/
void start_server(){
	struct sockaddr_in client_name;
	socklen_t client_name_len = sizeof(client_name);
	int port;

	signal(SIGINT,catch_ctrlc);

	port=Configurations->server_port;
	printf("Listening for HTTP requests on port %d\n",port);

	// Configure listening port
	if ((socket_conn=fireup(port))==-1)
		exit(1);

	// Serve requests
	while (1)
	{
		// Accept connection on socket
		if ((new_conn = accept(socket_conn,(struct sockaddr *)&client_name,&client_name_len)) == -1 ) {
			printf("Error accepting connection\n");
			exit(1);
		}
		current_requests++;
		var_condition++;
		buffer_of_requests->total_requests++;
		//signal que chegou um request
		śem_post(&sem_threads);
	//	pthread_cond_signal(&threads_cond_var);

		/* ver caso estaja cheia ai tem de ficar a espera de um sinal (need to create another variable condicion)
		if(current_requests<Configurations->thread_pool_size){

		}else{

		}
		*/

	}
}
void server_stats(){
	printf("INITIALIZED SERVER STATS\n" );
	Statistics = (Stats)malloc(sizeof(struct stats));
	Statistics->number_static_pages=0;
	Statistics->number_compressed_files=0;
	Statistics->average_time_content=0;
	Statistics->average_time_compressed_content=0;
	/*struct stats{
	  int number_static_pages, number_compressed_files;
	  float average_time_content, average_time_compressed_content;
	}*/
}
void Load_Configs(){
	current_requests=0;
	changing_policy=0;
	char* line = (char*)malloc(sizeof(char)*SIZE_NAME);
	Configurations = (struct server_configs*)malloc(sizeof(struct server_configs));
	FILE *file;
	file=fopen("config.txt","r");
	(Configurations->server_port) = atoi(fgets(line,SIZE_NAME,file));
	strcpy(Configurations->policy,	fgets(line,SIZE_NAME,file));
	Configurations->thread_pool_size = atoi(fgets(line,SIZE_NAME,file));
	strcpy((Configurations->allowed),(fgets(line,SIZE_NAME,file)));
	free(line);
	fclose(file);
}
void print_Configs(){
	printf("SERVER PORT : %i\n",Configurations->server_port);
	printf("POLICY : %s",Configurations->policy);
	printf("THREAD POOL SIZE : %i\n", Configurations->thread_pool_size);
	printf("EXTENSIONS ALLOWED : %s",Configurations->allowed);
}
// Processes request from client
void get_request(int socket)
{
	int i,j;
	int found_get;

	found_get=0;
	while ( read_line(socket,SIZE_BUF) > 0 ) {
		if(!strncmp(buf,GET_EXPR,strlen(GET_EXPR))) {
			// GET received, extract the requested page/script
			found_get=1;
			i=strlen(GET_EXPR);
			j=0;
			while( (buf[i]!=' ') && (buf[i]!='\0') )
				req_buf[j++]=buf[i++];
			req_buf[j]='\0';
		}
	}	

	// Currently only supports GET 
	if(!found_get) {
		printf("Request from client without a GET\n");
		exit(1);
	}
	// If no particular page is requested then we consider htdocs/index.html
	if(!strlen(req_buf))
		sprintf(req_buf,"index.html");

	#if DEBUG
	printf("get_request: client requested the following page: %s\n",req_buf);
	#endif

	return;
}
void create_thread_pool(){
	int i;
	//allocate memory for threads pool
	threads = malloc(Configurations->thread_pool_size*sizeof(pthread_t));
	//Create threads
	for(i=0;i<Configurations->thread_pool_size;i++){
		pthread_create(&threads[i],NULL,threads_handler,NULL);
		//printf("THREAD nº %i CREATED\n", (i+1));
	}
	printf("THREAD POOL CREATED\n");
}
// Send message header (before html page) to client
void send_header(int socket){
	#if DEBUG
	printf("send_header: sending HTTP header to client\n");
	#endif
	sprintf(buf,HEADER_1);
	send(socket,buf,strlen(HEADER_1),0);
	sprintf(buf,SERVER_STRING);
	send(socket,buf,strlen(SERVER_STRING),0);
	sprintf(buf,HEADER_2);
	send(socket,buf,strlen(HEADER_2),0);
	return;
}
// Execute script in /cgi-bin
void execute_script(int socket){
	// Currently unsupported, return error code to client
	cannot_execute(socket);
	return;
}
// Send html page to client
void send_page(int socket,int id_request){
	FILE * fp;

	// Searchs for page in directory htdocs

	sprintf(buffer_of_requests->list_requests[id_request].buffer_temp,"htdocs/%s",buffer_of_requests->list_requests[id_request].file_name);

	#if DEBUG
	printf("send_page: searching for %s\n",buffer_of_requests->list_requests[id_request].buffer_temp);
	#endif

	// Verifies if file exists
	if((fp=fopen(buffer_of_requests->list_requests[id_request].buffer_temp,"rt"))==NULL) {
		// Page not found, send error to client
		printf("send_page: page %s not found, alerting client\n",buffer_of_requests->list_requests[id_request].buffer_temp);
		not_found(socket);
	}
	else {
		// Page found, send to client

		// First send HTTP header back to client
		send_header(socket);

		printf("send_page: sending page %s to client\n",buffer_of_requests->list_requests[id_request].buffer_temp);
		while(fgets(buffer_of_requests->list_requests[id_request].buffer_temp,SIZE_BUF,fp))
			send(socket,buffer_of_requests->list_requests[id_request].buffer_temp,strlen(buffer_of_requests->list_requests[id_request].buffer_temp),0);

		// Close file
		fclose(fp);
	}

	return;
}
// Identifies client (address and port) from socket
void identify(int socket)
{
	char ipstr[INET6_ADDRSTRLEN];
	socklen_t len;
	struct sockaddr_in *s;
	int port;
	struct sockaddr_storage addr;

	len = sizeof addr;
	getpeername(socket, (struct sockaddr*)&addr, &len);

	// Assuming only IPv4
	s = (struct sockaddr_in *)&addr;
	port = ntohs(s->sin_port);
	inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);

	printf("identify: received new request from %s port %d\n",ipstr,port);

	return;
}
// Reads a line (of at most 'n' bytes) from socket
int read_line(int socket,int n){
	int n_read;
	int not_eol;
	int ret;
	char new_char;

	n_read=0;
	not_eol=1;

	while (n_read<n && not_eol) {
		ret = read(socket,&new_char,sizeof(char));
		if (ret == -1) {
			printf("Error reading from socket (read_line)");
			return -1;
		}
		else if (ret == 0) {
			return 0;
		}
		else if (new_char=='\r') {
			not_eol = 0;
			// consumes next byte on buffer (LF)
			read(socket,&new_char,sizeof(char));
			continue;
		}
		else {
			buf[n_read]=new_char;
			n_read++;
		}
	}

	buf[n_read]='\0';
	#if DEBUG
	printf("read_line: new line read from client socket: %s\n",buf);
	#endif

	return n_read;
}
// Creates, prepares and returns new socket
int fireup(int port){
	int new_sock;
	struct sockaddr_in name;

	// Creates socket
	if ((new_sock = socket(PF_INET, SOCK_STREAM, 0))==-1) {
		printf("Error creating socket\n");
		return -1;
	}

	// Binds new socket to listening port
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(new_sock, (struct sockaddr *)&name, sizeof(name)) < 0) {
		printf("Error binding to socket\n");
		return -1;
	}

	// Starts listening on socket
	if (listen(new_sock, 5) < 0) {
		printf("Error listening to socket\n");
		return -1;
	}

	return(new_sock);
}
// Sends a 404 not found status message to client (page not found)
void not_found(int socket){
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
// Send a 5000 internal server error (script not configured for execution)
void cannot_execute(int socket){
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
void terminate_threads(){
	int i;
	//thread_cond_broadcast(&threads_cond_var);
	/* Wait for all threads to complete */
	for(i=0;i<Configurations->thread_pool_size;i++){
	//printf("THREAD Nº %i TERMINDATED\n",i);
		pthread_cancel(threads[i]);
	}
}
// Closes socket before closing
void catch_ctrlc(int sig){
	//terminate_threads
	terminate_threads();
	printf("\nTHREADS TERMIDATED\n");

	//terminate thread mutex
	printf("MUTEX SEMAPHORE REALEASED\n");
	pthread_mutex_destroy(&simple_mutex);

	//releases the semaphore
	printf("SEMAPHORE REALEASED\n");
	sem_destroy(&sem_threads);

	//destroy condition variable
	pthread_cond_destroy(&threads_cond_var);
	printf("SHARED MEMORY REMOVED\n");

	//clean shared memory
	shmctl(shared_memory_id,IPC_RMID,NULL);

	//free structs
	free(threads);
	printf("THREADS \"MEMORY\" REMOVED\n");
	free(Configurations);
	printf("CONFIGURATIONS MEMORY REMOVED\n");
	free(buffer_of_requests->list_requests);
	free(buffer_of_requests);
	printf("BUFFER_REQUESTS MEMORY REMOVED\n");
	
	//close socket
	close(socket_conn);
	printf("SOCKET CLOSED\n");
	while(wait(NULL) != -1){
		printf("CHILD PROCESSE TERMINATED\n");
	}
	printf("SERVER TERMINATED\n");
	exit(0);
}