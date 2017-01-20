/*
* -- simplehttpd.c --
* A (very) simple HTTP server
*       gcc simplehttpd.c -lpthread -D_REENTRANT -Wall -o simplehttpds
* Sistemas Operativos 2016/2017xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
*/
/*
+ Arranque do servidor e aplicação das configurações existentes no ficheiro “config.txt” -> Done
+ Criação de todos os processo filho necessários -> Done
+ Criação da memória partilhada -> Done
+ Criação da pool de threads -> Done

+ Criação do named pipe -> Done
+ Leitura correcta dos comandos recebidos pelo named pipe -> Done
+ Aplicação das configurações recebidas pelo named pipe -> Done

+ Suporte do scheduler e política de escalonamento -> Done
- Suporte de concorrência no tratamento de pedidos -> NOT DONE ? que ito ?

+ Leitura correcta dos pedidos feitos por HTTP e a sua colocação no buffer de pedidos -> Done
+ Resposta a pedidos de páginas estáticas -> Done

+ Resposta a ficheiros comprimidos (extensão .gz) -> Done

+- Sincronização com mecanismos adequados (semáforos, mutexes ou variáveis de condição) -> Done need to see again
+- Prevenir interrupções indesejada por sinais e fornecer a resposta adequada aos vários sinais -> Done 80 %

+ Utilização da memória partilhada -> Done

+ Ler comandos do utilizador -> Done
+ Enviar comandos através do named pipe -> Done

+ Ler informação da memória partilhada -> Done
+ Escrever a informação estatística no ficheiro mapeado em memória (“server.log”) -> Done
+ Enviar informação agregada para o écran -> Done
+ Fazer reset a estatísticas agregadas -> Done

+ Deteção e tratamento de erros -> Done
+ Terminação dos processos filho quando o processo principal termina. Libertação de recursos e limpeza ao terminar a aplicação. -> Done

*/
/*
int fd;
    if ((fd = open(FILE_NAME,O_RDWR | O_CREAT,0600)) < 0){
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    PageSize = (int)sysconf(_SC_PAGESIZE);
    lseek( fd, PageSize-1, SEEK_SET);
 
    printf("<%d>\n", PageSize-1);
    // put something there
    write(fd, "a\0" , 2);
    printf("vai mapear mmap\n");
    //printf("st.st_size %d\n",st.st_size);
    //st.st_size = size;
    if ((str = mmap(0,PageSize,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0)) == MAP_FAILED){
        perror("Error in mmap");
        exit(EXIT_FAILURE);
    }
    close(fd);
 
escreve:
 
if ((fd = open(FILE_NAME,O_RDWR | O_CREAT,0600)) < 0){
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    printf("abriu fich escrita\n");
    offset += sprintf( str + offset, "%d\n", buffer[buffer_id-1].tipo);
    printf("tipo: <%s>\n", str);
    offset += sprintf(str + offset, "%s\n", buffer[buffer_id-1].request);
    printf("request: <%s>\n",str);
    offset += sprintf(str + offset, "%d:%d:%d:%d\n", buffer[buffer_id-1].tempo.horas,buffer[buffer_id-1].tempo.minutos,buffer[buffer_id-1].tempo.segundos,buffer[buffer_id-1].tempo.milisegundos);
    printf("tempo inicial: <%s>\n",str);
    offset += sprintf(str + offset, "%d:%d:%d:%d\n", h,m,s,ms);
    printf("tempo final: <%s>\n",str);
   
    close(fd);
*/
#include "headers.h"
#include "Stats.c"
#include "Server.c"
#include "Threads.c"

int main(int argc, char ** argv){
	/*static struct sigaction act;

//setting to my function catcher

	act.sa_handler = catch_ctrlc;       
	act.sa_flags    = 0;

	sigfillset(&(act.sa_mask)); 

	sigaction( SIGTERM, &act, NULL );
	sigaction( SIGKILL  , &act, NULL );*/


	signal(SIGINT,catch_ctrlc);
	signal(SIGTSTP,catch_ctrlz);

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
	buffer_of_requests->end_request=0;
	buffer_of_requests->current_requests=0;
	buffer_of_requests->present_request=0;
	buffer_of_requests->temp_current_requests=0;
		//create Processes
	process_maker=fork();
	if(process_maker==0)
	{
		//process child for server Statistics
		printf("pid son: %d \n\n",getpid());
		server_stats();
		exit(0);
	}
	else
	{
		//thread for the pipe listener
		pthread_create(&pipe_thread,NULL,create_named_pipe,NULL);
		//process father principal
		printf("pid father: %d \n",getpid());
		//Create Thread pool
		create_thread_pool();
		pthread_create(&scheduler_thread,NULL,scheduler_handler,NULL);
		// to start http server
		start_server();
		pthread_join(pipe_thread, NULL);
		pthread_join(scheduler_thread, NULL);
		exit(0);
	}
}
int isAllowed(char* file){
	int i;
	char temp[SIZE_BUF],temp2[SIZE_BUF];
	strcpy(temp2,file);
	for(i=0;i<MAX_FILES;i++){
		strcpy(temp,Configurations->allowed[i]);
		if(strcmp(temp,temp2)==0){
			return 1;
		}
	}
	return 0;
}
void catch_ctrlz(){
	if(process_maker!=0){
		int i;
		char attended[4];
		printf("\n\\------Requests list------/\n");
		for(i=0; i<buffer_of_requests->end_request; i++){
			buffer_of_requests->list_requests[i].attended ? strcpy(attended,"yes") : strcpy(attended,"no");
			printf("Request nº %d\nSocket - %d\nPort - %d\nId - %d\nFile name - %s\nAttended - %s\nEntry time - %sType - %d\n\n",i+1,buffer_of_requests->list_requests[i].socket,buffer_of_requests->list_requests[i].port,buffer_of_requests->list_requests[i].id,buffer_of_requests->list_requests[i].file_name,attended,ctime(&buffer_of_requests->list_requests[i].entry_time),buffer_of_requests->list_requests[i].type);
		}
		printf("\\-----------end-----------/\n");
	}
}
void start_server(){
	struct sockaddr_in client_name;
	socklen_t client_name_len = sizeof(client_name);
	int port=Configurations->server_port;
	printf("Listening for HTTP requests on port %d\n",port);
	// Configure listening port
	if ((socket_conn=fireup(port))==-1)
		exit(1);
	// Serve requests
	//sleep(10);
	while (1)
	{
		// Accept connection on socket
		if ((new_conn = accept(socket_conn,(struct sockaddr *)&client_name,&client_name_len)) == -1 ){
			printf("Error accepting connection\n");
			exit(1);
		}
		pthread_mutex_lock(&mutex_buffer);

		if(++buffer_of_requests->end_request==MAX_REQUESTS) buffer_of_requests->end_request = 0;
		
		buffer_of_requests->list_requests[buffer_of_requests->end_request].socket = new_conn;
		time(&buffer_of_requests->list_requests[buffer_of_requests->end_request].entry_time);
		gettimeofday(&buffer_of_requests->list_requests[buffer_of_requests->end_request].tval_entry, NULL);
		printf("\nEntry time %s",ctime(&buffer_of_requests->list_requests[buffer_of_requests->end_request].entry_time));

		total_requests++;
		buffer_of_requests->current_requests++;
		buffer_of_requests->list_requests[buffer_of_requests->end_request].id =total_requests;
		
		int index = buffer_of_requests->end_request;
			//unblocked mutex in the begining of the identify
		identify(index);
			//signal to add request to buffer
		
		pthread_cond_signal(&threads_cond_var);
	}
}
int bigger_date(time_t date_1,time_t date_2){
	double seconds = difftime(date_1, date_2);
	if(seconds > 0)
		return 1;
	return 0;
}
// Identifies client (address and port) from socket
void identify(int index_in_buffer){
	pthread_mutex_unlock(&mutex_buffer);
	char ipstr[INET6_ADDRSTRLEN];	
	socklen_t len;
	struct sockaddr_in *s;
	int port;
	struct sockaddr_storage addr;
	len = sizeof addr;
	getpeername(buffer_of_requests->list_requests[index_in_buffer].socket, (struct sockaddr*)&addr, &len);

	// Assuming only IPv4
	s = (struct sockaddr_in *)&addr;
	port = ntohs(s->sin_port);
	buffer_of_requests->list_requests[index_in_buffer].port = port;
	inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	strcpy(buffer_of_requests->list_requests[index_in_buffer].ip_str, ipstr);
	printf("identify: received new request from %s port %d\n",buffer_of_requests->list_requests[index_in_buffer].ip_str,port);
	// Process request
	get_request(index_in_buffer);
	return;
}
double get_duration(int index){
	char str_time[SIZE_BUF];
	double ret;
	struct timeval tval_after, tval_result;
	gettimeofday(&tval_after, NULL);
	timersub(&tval_after, &buffer_of_requests->list_requests[index].tval_entry, &tval_result);
	sprintf(str_time,"%ld.%06ld", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
	sscanf(str_time, "%lf", &ret);
	return ret;
}
void handle_response(int index){
	pthread_mutex_unlock(&mutex_buffer);
	struct stats * st_m;
	time_t t_m;

	if(isAllowed(buffer_of_requests->list_requests[index].file_name)){
		// Verify if request is for a page or script
		buffer_of_requests->list_requests[index].type==2 ? execute_script(index): send_page(index);
		//time
	}else{
		strcpy(buffer_of_requests->list_requests[index].file_name,"not_allowed.html");
		send_page(index);
	}
	get_duration(index);
	
	//stats
	sem_wait(&mutex_stats);
	
	if((st_m = (Stats)shmat(shared_memory_id,NULL,0))==(Stats)-1){
		perror("ERROR ATTACHING MEMORY");
		exit(1);
	}
	
	//st_m= (struct stats *) shmat(shared_memory_id,NULL,0);

	strcpy(st_m->arrival,ctime(&buffer_of_requests->list_requests[index].entry_time));
	time(&t_m);
	strcpy(st_m->handled,ctime(&t_m));
	strcpy(st_m->file_name,buffer_of_requests->list_requests[index].file_name);
	st_m->request_type= buffer_of_requests->list_requests[index].type;
	st_m->duration=get_duration(index);

	//detach
	shmdt(Statistics);
	sem_post(&mutex_stats);
	//send signal to update file with new entry
	kill(process_maker,SIGTSTP);
	// Terminate connection with client
	buffer_of_requests->list_requests[index].attended=1;
	printf("Attended with policy %s\n",Configurations->policy);
}
void Load_Configs(){
	int i=0;
	changing_policy=0;
	char * pch;
	char* line = (char*)malloc(sizeof(char)*SIZE_NAME);
	Configurations = (struct server_configs*)malloc(sizeof(struct server_configs));
	FILE *file;
	file=fopen("config.txt","r");
	(Configurations->server_port) = atoi(fgets(line,SIZE_NAME,file));
	strcpy(Configurations->policy,	fgets(line,SIZE_NAME,file));
	strtok(Configurations->policy, "\n");
	printf("Configurations %s \n",Configurations->policy );
	Configurations->thread_pool_size = atoi(fgets(line,SIZE_NAME,file));
	fgets(line,SIZE_NAME,file);
	pch = strtok (line,";");
	while (pch != NULL)
	{
		strcpy((Configurations->allowed[i++]),pch);
		pch = strtok (NULL, ";");
	}
	strcpy((Configurations->allowed[i]),"eof");
	free(line);
	fclose(file);
}
void print_Configs(){
	int i=0;
	printf("\\------Configurations------/\n");
	printf("SERVER PORT : %i\n",Configurations->server_port);
	printf("POLICY : %s\n",Configurations->policy);
	printf("THREAD POOL SIZE : %i\n", Configurations->thread_pool_size);
	printf("EXTENSIONS ALLOWED :");
	while(strcmp(Configurations->allowed[i],"eof")!=0)
		printf(" %s", Configurations->allowed[i++]);
	printf("\\-------------------------/\n\n");
}
//see if file is static or not 1 - static 2- compressed 0 - no one but probably html XD
int get_file_type(int index_in_buffer){
	if(strstr(buffer_of_requests->list_requests[index_in_buffer].file_name,".html") != NULL){
		printf("File type Static\n");
		return 1;
	}
	else if(strstr(buffer_of_requests->list_requests[index_in_buffer].file_name,".gz") != NULL){
		printf("File type Compressed\n");
		return 2;
	}
	else{
		return 1;
	}
}
// Execute script in /cgi-bin
void execute_script(int index) {
	FILE *in;
	int socket = buffer_of_requests->list_requests[index].socket;
	char buf[512];

	char cmd[SIZE_BUF];
	strcpy(cmd,"gunzip -c htdocs/");
	strcat(cmd,buffer_of_requests->list_requests[index].file_name);

	if((in = popen(cmd, "r")) == NULL) {
		not_found(index);
	}	else {
		send_header(index);
		while (fgets(buf, sizeof(buf) - 1, in) != NULL) {
			send(socket, buf, strlen(buf), 0); 
		}
	}
	pclose(in);
	close(socket);
}

// Send html page to client
void send_page(int index_in_buffer){
	FILE * fp;
	// Searchs for page in directory htdocs
	sprintf(buffer_of_requests->list_requests[index_in_buffer].buf_temp,"htdocs/%s",buffer_of_requests->list_requests[index_in_buffer].file_name);
		#if DEBUG
	printf("send_page: searching for %s\n",buffer_of_requests->list_requests[index_in_buffer].buf_temp);
		#endif
	// Verifies if file exists
	if((fp=fopen(buffer_of_requests->list_requests[index_in_buffer].buf_temp,"rt"))==NULL){
		// Page not found, send error to client
		printf("send_page: page %s not found, alerting client\n",buffer_of_requests->list_requests[index_in_buffer].buf_temp);
		not_found(index_in_buffer);
	}
	else
	{
		// Page found, send to client
		// First send HTTP header back to client
		send_header(index_in_buffer);
		printf("send_page: sending page %s to client\n",buffer_of_requests->list_requests[index_in_buffer].buf_temp);
		while(fgets(buffer_of_requests->list_requests[index_in_buffer].buf_temp,SIZE_BUF,fp))
			send(buffer_of_requests->list_requests[index_in_buffer].socket,buffer_of_requests->list_requests[index_in_buffer].buf_temp,strlen(buffer_of_requests->list_requests[index_in_buffer].buf_temp),0);
		// Close file
		fclose(fp);
	}

	close(buffer_of_requests->list_requests[index_in_buffer].socket);
	return;
}
void reorganize_buffer(){
	//bubble sort
	pthread_mutex_lock(&mutex_buffer);

	int  n, c, d;
	struct request swap;
	if(strcmp(Configurations->policy,"NORMAL")==0)
	{
		n = buffer_of_requests->end_request;
		for (c = 0 ; c < ( n - 1 ); c++)
		{
			for (d = 0 ; d < n - c - 1; d++)
			{
		if (bigger_date(buffer_of_requests->list_requests[d].entry_time , buffer_of_requests->list_requests[d+1].entry_time)) /* For decreasing order use < */
				{
					swap  = buffer_of_requests->list_requests[d];
					buffer_of_requests->list_requests[d] = buffer_of_requests->list_requests[d+1];
					buffer_of_requests->list_requests[d+1]= swap;
				}
			}
		}
	}
	else if(strcmp(Configurations->policy,"STATIC")==0)
	{
		int i;
		int j;
		int m;
		struct request aux;
		for(i=buffer_of_requests->present_request; i < buffer_of_requests->end_request; i++)
		{
			if(buffer_of_requests->list_requests[i].type==2)
			{
				for(j=i+1; j<buffer_of_requests->end_request; j++)
				{
					if(buffer_of_requests->list_requests[j].type==1)
					{
						for(m=j; m>i; m--)
						{
							aux=buffer_of_requests->list_requests[m];
							buffer_of_requests->list_requests[m] = buffer_of_requests->list_requests[m-1];
							buffer_of_requests->list_requests[m-1]= aux;
						}
						break;
					}
				}
			}
		}

	}
	else if(strcmp(Configurations->policy,"COMPRESSED")==0){
		int i;
		int j;
		int m;
		struct request aux;
		for(i=buffer_of_requests->present_request; i < buffer_of_requests->end_request; i++)
		{
			if(buffer_of_requests->list_requests[i].type==1)
			{
				for(j=i+1; j<buffer_of_requests->end_request; j++)
				{
					if(buffer_of_requests->list_requests[j].type==2)
					{
						for(m=j; m>i; m--)
						{
							aux=buffer_of_requests->list_requests[m];
							buffer_of_requests->list_requests[m] = buffer_of_requests->list_requests[m-1];
							buffer_of_requests->list_requests[m-1]= aux;
						}
						break;
					}
				}
			}
		}
	}
	else
	{
		printf("Policy ?\n");
	}
	changing_policy=0;
	pthread_mutex_unlock(&mutex_buffer);
}
void create_thread_pool(){
	int i;
	//allocate memory for threads pool
	threads = malloc((Configurations->thread_pool_size)*sizeof(pthread_t));
	//Create threads
	for(i=0; i<Configurations->thread_pool_size; i++)
		pthread_create(&threads[i],NULL,worker,NULL);
	printf("Thread pool created\n");

	/*for(i=0; i<Configurations->thread_pool_size; i++){
		pthread_join(threads[i],NULL);
	}*/
	
}
void terminate_threads(){
	int i;
	pthread_kill(scheduler_thread,SIGUSR1);
	pthread_kill(pipe_thread, SIGUSR1);
	for(i=0; i<Configurations->thread_pool_size; i++)
		pthread_kill(threads[i], SIGUSR2);
}
// Closes socket before closing
void catch_ctrlc(){
	if(process_maker!=0){
		//terminate_threads
		kill(process_maker,SIGINT);
		terminate_threads();
		printf("\nThreads termidated\n");

		//terminate thread mutex
		if(pthread_mutex_destroy(&mutex_buffer) == 0)
			printf("Mutex 1 ");
		else
			printf("Problem on mutex semaphore realease 1\n");

		pthread_mutex_unlock(&handler_mutex);
		if(pthread_mutex_destroy(&handler_mutex) == 0)
			printf(" 2 released\n");
		else
			printf("Problem on mutex semaphore realease 2\n");

		//destroy condition variable
		pthread_cond_destroy(&threads_cond_var);
		printf("Condition variable destroyed\n");

		//close socket
		if(close(socket_conn)==0)
			printf("Socket closed\n");
		else
			printf("Problem on socket closing\n");
		close(new_conn);

		//releases the semaphore
		if(sem_destroy(&sem_threads) == 0 && sem_destroy(&mutex_stats) == 0 )
			printf("Semaphore destroyed\n");
		else
			printf("Error on semaphore realease\n");

		sleep(1);
		//free structs
		free(threads);
		free(Configurations);
		free(buffer_of_requests->list_requests);
		free(buffer_of_requests);
		printf("Server terminated\n");
		exit(0);
	}
}
void create_semaphores(){
	sem_init(&sem_threads,0,1);
	sem_wait(&sem_threads);
	sem_init(&mutex_stats,0,1);
}
