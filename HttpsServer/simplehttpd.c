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

+ Suporte do scheduler e política de escalonamento -> NOT DONE ? que ito ?
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
#include "headers.h"
#include "Stats.c"
int main(int argc, char ** argv)
{

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
	buffer_of_requests->end_temp_policy=0;
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
		pthread_create(&pipe_thread,NULL,create_named_pipe,NULL);
        //process father principal
		printf("pid father: %d \n",getpid());
        //Create Thread pool
		create_thread_pool();
		pthread_create(&scheduler_thread,NULL,scheduler_handler,NULL);
        // to start http server
		start_server();
		pthread_join(scheduler_thread, NULL);
		pthread_join(pipe_thread, NULL);
		exit(0);
	}
}
int isAllowed(char* file)
{
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
void clean_named_pipe()
{
	printf("\n\nNamed pipe thread clean\n");
	pthread_exit(NULL);
}
void *create_named_pipe(){
	signal(SIGUSR2, clean_named_pipe);
	int i=0;
	char * pch;
    // Creates the named pipe if it doesn't exist yet
	if ((mkfifo(PIPE_NAME, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST))
	{
		perror("Cannot create pipe: ");
		exit(0);
	}
	printf("Named Pipe: %s created\n",PIPE_NAME);
    // Opens the pipe for reading
	int fd;
	if ((fd=open(PIPE_NAME, O_RDWR)) < 0)
	{
		perror("Cannot open pipe for reading: ");
		exit(0);
	}
	command cmd;
	while (1)
	{
		read(fd, &cmd, sizeof(command));
        //clear_msg(buf,final,arg2);
		printf("Received: %d + thread_size %d + policy %d + files allowed: %s\n", cmd.cmd,cmd.thread_size,cmd.policy,cmd.str_1);
		switch(cmd.cmd)
		{
			case 1:
			printf("Buffer reorganizing.... \n");
			changing_policy=1;
			switch(cmd.policy)
			{
				case 1: strcpy(Configurations->policy,"NORMAL"); break;
				case 2: strcpy(Configurations->policy,"STATIC"); break;
				case 3: strcpy(Configurations->policy,"COMPRESSED"); break;
			}
			reorganize_buffer(); 

			printf("Buffer is organized \n");
			break;
			case 2:
			if(Configurations->thread_pool_size<cmd.thread_size)
			{
                //aumentar
				threads = realloc(threads,cmd.thread_size*sizeof(pthread_t));
				for(i=Configurations->thread_pool_size; i<cmd.thread_size; i++)
					pthread_create(&threads[i],NULL,worker,NULL);
				Configurations->thread_pool_size=cmd.thread_size;
			}
			else if(Configurations->thread_pool_size>cmd.thread_size)
			{
                //reduzir
				for(i=cmd.thread_size ; i<Configurations->thread_pool_size ; i++)
					pthread_kill(threads[i], SIGUSR1);
				Configurations->thread_pool_size=cmd.thread_size;
				threads = realloc(threads,Configurations->thread_pool_size*sizeof(pthread_t));
			}
			else
				printf("No need to do anything to thread pool size\n");

			printf("Thread pool have now %d threads\n",Configurations->thread_pool_size);
			break;
			case 3:
			i=0;
			pch = strtok (cmd.str_1,";");
			while (pch != NULL)
			{
				strcpy((Configurations->allowed[i++]),pch);
				pch = strtok (NULL, ";");
			}
			strcpy((Configurations->allowed[i]),"eof");
            //printf("Not done \n");
			print_Configs();
			break;
			default:
			printf("Invalid command\n");
		}
	}
}
void catch_ctrlz(){
	int i;
	char attended[4];
	printf("\n\\------Requests list------/\n");
	for(i=0; i<buffer_of_requests->end_request; i++)
	{
		buffer_of_requests->list_requests[i].attended ? strcpy(attended,"yes") : strcpy(attended,"no");
		printf("Request nº %d\nSocket - %d\nPort - %d\nId - %d\nFile name - %s\nAttended - %s\nEntry time - %sType - %d\n\n",i+1,buffer_of_requests->list_requests[i].socket,buffer_of_requests->list_requests[i].port,buffer_of_requests->list_requests[i].id,buffer_of_requests->list_requests[i].file_name,attended,ctime(&buffer_of_requests->list_requests[i].entry_time),buffer_of_requests->list_requests[i].type);
	}
	printf("\\-----------end-----------/\n");
}
void start_server()
{
	struct sockaddr_in client_name;
	socklen_t client_name_len = sizeof(client_name);
	int port;

	port=Configurations->server_port;
	printf("Listening for HTTP requests on port %d\n",port);
    // Configure listening port
	if ((socket_conn=fireup(port))==-1)
		exit(1);
    // Serve requests
	while (1)
	{
        // Accept connection on socket
		if ((new_conn = accept(socket_conn,(struct sockaddr *)&client_name,&client_name_len)) == -1 )
		{
			printf("Error accepting connection\n");
			exit(1);
		}
		buffer_of_requests->list_requests[buffer_of_requests->end_request].socket = new_conn;
		time(&buffer_of_requests->list_requests[buffer_of_requests->end_request].entry_time);
		gettimeofday(&buffer_of_requests->list_requests[buffer_of_requests->end_request].tval_entry, NULL);

		printf("\nEntry time %s",ctime(&buffer_of_requests->list_requests[buffer_of_requests->end_request].entry_time));
		total_requests++;
		//changing_policy ? buffer_of_requests->temp_current_requests++ : buffer_of_requests->current_requests++;
		buffer_of_requests->current_requests++;
		buffer_of_requests->list_requests[buffer_of_requests->end_request].id =total_requests;
		if(total_requests<=MAX_REQUESTS)
		{
			pthread_mutex_lock(&mutex_buffer);
			int index = buffer_of_requests->end_request;
			buffer_of_requests->end_request++;
            //unblocked mutex in the begining of the identify
			identify(index);
			//reorganize_buffer();
            //signal to add request to buffer
			pthread_cond_signal(&threads_cond_var);
		}
		else
			printf("This server can only answer %d",MAX_REQUESTS);
	}
}
void clean_scheduler_handler()
{
	printf("Scheduler handler clean\n");
	pthread_exit(NULL);
}
void *scheduler_handler()
{
	signal(SIGUSR2, clean_scheduler_handler);
	int flag=1;
	while(1)
	{
		pthread_mutex_lock(&handler_mutex);
		while(buffer_of_requests->current_requests==0)
			pthread_cond_wait(&threads_cond_var,&handler_mutex);
		pthread_mutex_unlock(&handler_mutex);
		if(buffer_of_requests->current_requests<Configurations->thread_pool_size && !changing_policy)
		{
			printf("Thread freed to attend request\n");
			buffer_of_requests->current_requests--;
			reorganize_buffer();
			sem_post(&sem_threads);
		}
		else if(changing_policy)
		{
			!flag ? printf(" "): printf("Please wait changing policy...\n") ;
			flag=0;
		}
		else
		{
			!flag ? printf(" "): printf("Full please wait ...\n");
			flag=0;
		}
	}
	return 0;

}

int bigger_date(time_t date_1,time_t date_2)
{
	double seconds = difftime(date_1, date_2);

	if(seconds > 0)
		return 1;
	return 0;
}

// Identifies client (address and port) from socket
void identify(int index_in_buffer)
{
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
int get_index_request()
{
    //return the index of the lowest id
	int i,index=-1,min=0;
	for(i=buffer_of_requests->present_request; i<buffer_of_requests->end_request; i++)
	{
		if(buffer_of_requests->list_requests[i].id<=min)
		{
			min = buffer_of_requests->list_requests[i].id;
			index=i;
		}
	}
	return index;
}
void clean_worker()
{
	printf("Worker clean\n");
	pthread_exit(0);
}
void * worker()
{
	signal(SIGUSR1, clean_worker);
	while(1)
	{
		sem_wait(&sem_threads);
		changing_policy ? printf("Changing policy\n") : printf("\n");
		pthread_mutex_lock(&mutex_buffer);
		int index = buffer_of_requests->present_request++;
		handle_response(index);
	}
    // Identify new client
	pthread_exit(NULL);
	return 0;
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
void handle_response(int index)
{
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
	st_m= (struct stats *) shmat(shared_memory_id,NULL,0);
	strcpy(st_m->arrival,ctime(&buffer_of_requests->list_requests[index].entry_time));
	time(&t_m);
	strcpy(st_m->handled,ctime(&t_m));
	strcpy(st_m->file_name,buffer_of_requests->list_requests[index].file_name);
	st_m->request_type= buffer_of_requests->list_requests[index].type;
	st_m->duration=get_duration(index);
	shmdt(Statistics);
	sem_post(&mutex_stats);

    //send signal to update file with new entry
	kill(process_maker,SIGUSR1);
    // Terminate connection with client
	buffer_of_requests->list_requests[index].attended=1;
	printf("Attended with policy %s\n",Configurations->policy);
}
void Load_Configs()
{
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
void print_Configs()
{
	int i=0;
	printf("\\------Configurations------/\n");
	printf("SERVER PORT : %i\n",Configurations->server_port);
	printf("POLICY : %s",Configurations->policy);
	printf("THREAD POOL SIZE : %i\n", Configurations->thread_pool_size);
	printf("EXTENSIONS ALLOWED :");
	while(strcmp(Configurations->allowed[i],"eof")!=0)
		printf(" %s", Configurations->allowed[i++]);

	printf("\\-------------------------/\n\n");
}
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
//see if file is static or not 1 - static 2- compressed 0 - no one
int get_file_type(int index_in_buffer)
{
	if(strstr(buffer_of_requests->list_requests[index_in_buffer].file_name,".html") != NULL)
	{
		printf("File type Static\n");
		return 1;
	}
	else if(strstr(buffer_of_requests->list_requests[index_in_buffer].file_name,".gz") != NULL)
	{
		printf("File type Compressed\n");
		return 2;
	}
	else
	{
		return 1;
	}
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
// Execute script in /cgi-bin
void execute_script(int index_in_buffer)
{
	char cmd[SIZE_BUF];
	strcpy(cmd,"gzip -d htdocs/");
	strcat(cmd,buffer_of_requests->list_requests[index_in_buffer].file_name);
	system(cmd);
    //remove extencion
	strtok(buffer_of_requests->list_requests[index_in_buffer].file_name,".");
	send_page(index_in_buffer);

	return;
}
// Send html page to client
void send_page(int index_in_buffer)
{
	FILE * fp;
    // Searchs for page in directory htdocs
	sprintf(buffer_of_requests->list_requests[index_in_buffer].buf_temp,"htdocs/%s",buffer_of_requests->list_requests[index_in_buffer].file_name);
#if DEBUG
	printf("send_page: searching for %s\n",buffer_of_requests->list_requests[index_in_buffer].buf_temp);
#endif
	
    // Verifies if file exists
	if((fp=fopen(buffer_of_requests->list_requests[index_in_buffer].buf_temp,"rt"))==NULL)
	{
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
// Reads a line (of at most 'n' bytes) from socket
int read_line(int index_in_buffer,int n)
{
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
void reorganize_buffer()
{
	//printf(" %s\n",Configurations->policy);

    //bubble sort
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
}
void create_thread_pool()
{
	int i;
    //allocate memory for threads pool
	threads = malloc((Configurations->thread_pool_size)*sizeof(pthread_t));
    //Create threads
	for(i=0; i<Configurations->thread_pool_size; i++)
		pthread_create(&threads[i],NULL,worker,NULL);

	printf("Thread pool created\n");
}
// Creates, prepares and returns new socket
int fireup(int port)
{
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
// Sends a 404 not found status message to client (page not found)
void not_found(int index_in_buffer)
{
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
// Send a 5000 internal server error (script not configured for execution)
void cannot_execute(int index_in_buffer)
{
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
void terminate_threads()
{
	int i;
	pthread_kill(scheduler_thread,SIGUSR2);
	//pthread_kill(pipe_thread, SIGUSR2);
	for(i=0; i<Configurations->thread_pool_size; i++)
		pthread_kill(threads[i], SIGUSR1);
}
// Closes socket before closing
void catch_ctrlc(int sig)
{
	if(process_maker!=0){


    //terminate_threads
		terminate_threads();

		printf("\nThreads termidated\n");

    //terminate thread mutex
		if(pthread_mutex_destroy(&mutex_buffer) == 0)
			printf("MUTEX SEMAPHORE REALEASED 1\n");
		else
			printf("PROBLEM ON MUTEX SEMAPHORE REALEASE 1\n");


		pthread_mutex_unlock(&handler_mutex);
		if(pthread_mutex_destroy(&handler_mutex) == 0)
			printf("MUTEX SEMAPHORE REALEASED 2\n");
		else
			printf("PROBLEM ON MUTEX SEMAPHORE REALEASE 2\n");

    //destroy condition variable
		pthread_cond_destroy(&threads_cond_var);
		printf("Condition variable destroyed\n");

    //clean shared memory
		shmdt(&shared_memory_id);
		shmctl(shared_memory_id,IPC_RMID,NULL);
		printf("Shared memory cleaned\n");

    //close socket
		if(close(socket_conn)==0)
			printf("Socket closed\n");
		else
			printf("PROBLEM ON SOCKET CLOSING\n");
		close(new_conn);

    //releases the semaphore
		if(sem_destroy(&sem_threads) == 0 && sem_destroy(&mutex_stats) == 0 )
			printf("Semaphore destroyed\n");
		else
			printf("ERROR ON SEMAPHORE REALEASE\n");


    //por causa dos prints
		sleep(1);
		kill(process_maker,SIGINT);
    //free structs
		free(threads);
		free(Configurations);
		free(buffer_of_requests->list_requests);
		free(buffer_of_requests);

		printf("SERVER TERMINATED\n");
		exit(0);
	}else{
		printf("Sou o filho do \n");
	}
}

void create_semaphores()
{
	sem_init(&sem_threads,0,1);
	sem_wait(&sem_threads);
	sem_init(&mutex_stats,0,1);

    //sem_wait(&mutex_stats);


    //sem_getvalue(&sem_threads, &value);
    //printf("valor %d\n",value);
}
