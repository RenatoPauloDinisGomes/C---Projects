/*
* -- simplehttpd.c --
* A (very) simple HTTP server
*       gcc simplehttpd.c -lpthread -D_REENTRANT -Wall -o simplehttpds
* Sistemas Operativos 2016/2017xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
*/
/*
- Arranque do servidor e aplicação das configurações existentes no ficheiro “config.txt” -> Done
- Criação de todos os processo filho necessários -> Done
- Criação da memória partilhada -> Done
- Criação da pool de threads -> Done

- Criação do named pipe ->  DONE
- Leitura correcta dos comandos recebidos pelo named pipe -> DONE
- Aplicação das configurações recebidas pelo named pipe ->  DONE 80 %
- Suporte do scheduler e política de escalonamento -> NOT DONE
- Suporte de concorrência no tratamento de pedidos -> DONE

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
int main(int argc, char ** argv)
{
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
	}
}
void server_stats()
{
	printf("Initialized server stats\n" );
	Statistics = (Stats)malloc(sizeof(struct stats));
	Statistics->total_requests =0;
	Statistics->number_static_pages=0;
	Statistics->number_compressed_files=0;
	Statistics->average_time_content=0;
	Statistics->average_time_compressed_content=0;
}
int isAllowed(char* file)
{


	return 0;
}
void *create_named_pipe()
{
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
			changing_policy=1;
			switch(cmd.policy)
			{
				case 1:
				strcpy(Configurations->policy,"NORMAL");
				reorganize_buffer();
				break;
				case 2:
				strcpy(Configurations->policy,"STATIC");
				reorganize_buffer();
				break;
				case 3:
				strcpy(Configurations->policy,"COMPRESSED");
				reorganize_buffer();
				break;
			}
			break;
			case 2:
			if(Configurations->thread_pool_size<cmd.thread_size)
			{
                //aumentar
				threads = realloc(threads,cmd.thread_size*sizeof(pthread_t));
				for(i=Configurations->thread_pool_size; i<cmd.thread_size; i++)
				{
					pthread_create(&threads[i],NULL,worker,NULL);
				}
				Configurations->thread_pool_size=cmd.thread_size;
			}
			else if(Configurations->thread_pool_size>cmd.thread_size)
			{
                //reduzir
				for(i=cmd.thread_size ; i<Configurations->thread_pool_size ; i++)
				{
					if(pthread_cancel(threads[i])!=0)
					{
						printf("PROBLEM TERMINATING THREAD Nº %d\n",i);
					}
				}
                //	threads = malloc((Configurations->thread_pool_size)*sizeof(pthread_t));
				Configurations->thread_pool_size=cmd.thread_size;
				threads = realloc(threads,Configurations->thread_pool_size*sizeof(pthread_t));
			}
			else
			{
				printf("No need to do anything to thread pool size\n");
			}
			printf("Thread pool have now %d threads\n",Configurations->thread_pool_size);
            //Configurations->thread_pool_size=cmd.thread_size;
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
void catch_ctrlz()
{
    //printf("Entry time %s",ctime(&buffer_of_requests->list_requests[buffer_of_requests->end_request].entry_time));
    //int  attended, socket, port, id, type;
    //char file_name[SIZE_BUF],ip_str[INET6_ADDRSTRLEN],buf_aux[SIZE_BUF],buf_temp[SIZE_BUF];
	int i;
	char attended[4];
	printf("\n\\------Requests list------/\n");
	for(i=0; i<buffer_of_requests->end_request; i++)
	{
		buffer_of_requests->list_requests[i].attended ? strcpy(attended,"yes") : strcpy(attended,"no");
		printf("Request nº %d\nSocket - %d\nPort - %d\nId - %d\nFile name - %s\nAttended - %s\nEntry time - %s\n",i+1,buffer_of_requests->list_requests[i].socket,buffer_of_requests->list_requests[i].port,buffer_of_requests->list_requests[i].id,buffer_of_requests->list_requests[i].file_name,attended,ctime(&buffer_of_requests->list_requests[i].entry_time));
	}
	printf("\\-----------end-----------/\n");
}
void start_server()
{
	struct sockaddr_in client_name;
	socklen_t client_name_len = sizeof(client_name);
	int port;
	signal(SIGINT,catch_ctrlc);
	signal(SIGTSTP,catch_ctrlz);
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
		printf("Entry time %s",ctime(&buffer_of_requests->list_requests[buffer_of_requests->end_request].entry_time));
		Statistics->total_requests++;
		changing_policy ? buffer_of_requests->temp_current_requests++ : buffer_of_requests->current_requests++;
		buffer_of_requests->list_requests[buffer_of_requests->end_request].id = Statistics->total_requests;
		if(Statistics->total_requests<=MAX_REQUESTS)
		{
			printf("Request number %d is gonna be put now in the buffer",buffer_of_requests->list_requests[buffer_of_requests->end_request].id);
			printf("\n\n");
			pthread_mutex_lock(&variables_mutex);
			int index = buffer_of_requests->end_request;
			buffer_of_requests->end_request++;
            //unblocked mutex in the begining of the identify
			identify(index);
            //signal to add request to buffer
			pthread_cond_signal(&threads_cond_var);
		}
		else
		{
			printf("This server can only answer %d",MAX_REQUESTS);
		}
	}
}
void *scheduler_handler()
{
	int flag=1;
	while(1)
	{
		pthread_mutex_lock(&handler_mutex);
		while(buffer_of_requests->current_requests==0)
		{
			pthread_cond_wait(&threads_cond_var,&handler_mutex);
		}
		pthread_mutex_unlock(&handler_mutex);
		if(buffer_of_requests->current_requests<Configurations->thread_pool_size && !changing_policy)
		{
			printf("Thread freed to attend request\n");
			buffer_of_requests->current_requests--;
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
	if (seconds > 0)
	{
		printf("Date_1 > Date_2\n");
		return 1;
	}
	else
	{
		printf("Date_2 > Date_1\n");
		return 0;
	}
	return 0;
}

// Identifies client (address and port) from socket
void identify(int index_in_buffer)
{
	pthread_mutex_unlock(&variables_mutex);
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

void * worker()
{
	while(1)
	{
		sem_wait(&sem_threads);
		changing_policy ? printf("Changing policy\n") : printf(" ");
        //printf("ssss %d\n",changing_policy);
		pthread_mutex_lock(&variables_mutex);
        //int index =get_index_request();
		int index = buffer_of_requests->present_request++;
        //buffer_of_requests->present_request++;
		handle_response(index);
	}
    // Identify new client
	pthread_exit(NULL);
	return 0;
}
void handle_response(int index)
{
	pthread_mutex_unlock(&variables_mutex);
    // Verify if request is for a page or script
	if(!strncmp(buffer_of_requests->list_requests[index].file_name,CGI_EXPR,strlen(CGI_EXPR)))
		execute_script(index);
	else
        // Search file with html page and send to client
		send_page(index);
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
	{
		printf(" %s", Configurations->allowed[i++]);
	}
	printf("\\-------------------------/\n\n");
}
// Processes request from client
void get_request(int index_in_buffer)
{
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
	if(!strncmp(buffer_of_requests->list_requests[index_in_buffer].file_name,CGI_EXPR,strlen(CGI_EXPR)))
		buffer_of_requests->list_requests[index_in_buffer].type=1;
	else
        // Search file with html page and send to client
		buffer_of_requests->list_requests[index_in_buffer].type=2;
#if DEBUG
	printf("get_request: client requested the following page: %s\n",buffer_of_requests->list_requests[index_in_buffer].file_name);
#endif

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
// Execute script in /cgi-bin
void execute_script(int index_in_buffer)
{
    // Currently unsupported, return error code to client
	cannot_execute(index_in_buffer);
	close(buffer_of_requests->list_requests[index_in_buffer].socket);
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
		not_found(buffer_of_requests->list_requests[index_in_buffer].socket);
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
	printf("%s\n",Configurations->policy);

    //bubble sort
	int  n, c, d;
	struct request swap;
	printf("Buffer reorganizing.... \n");

	if(strcmp(Configurations->policy,"NORMAL")==0)
	{
		printf("1\n");
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

	}
	else if(strcmp(Configurations->policy,"COMPRESSED")==0)
	{

	}
	else
	{
		printf("Policy ?\n");
	}

	printf("Buffer is organized \n");
}
void create_thread_pool()
{
	int i;
    //allocate memory for threads pool
	threads = malloc((Configurations->thread_pool_size)*sizeof(pthread_t));
    //Create threads
	for(i=0; i<Configurations->thread_pool_size; i++)
	{
		pthread_create(&threads[i],NULL,worker,NULL);
        //printf("THREAD nº %i CREATED\n", (i+1));
	}
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
    /* Wait for all threads to complete */
	for(i=0; i<Configurations->thread_pool_size; i++)
	{
        //printf("THREAD Nº %i TERMINDATED\n",i);
		if(pthread_cancel(threads[i])!=0)
		{
			printf("PROBLEM TERMINATING THREAD Nº %d\n",i);
		}
	}
}
// Closes socket before closing
void catch_ctrlc(int sig)
{
    //terminate_threads
	terminate_threads();
	if(pthread_cancel(scheduler_thread)!=0)
	{
		printf("PROBLEM TERMINATING THREAD scheduler\n");
	}
	if(pthread_cancel(pipe_thread)!=0)
	{
		printf("PROBLEM TERMINATING THREAD pipe_thread\n");
	}
	printf("\nTHREADS TERMIDATED\n");

    //terminate thread mutex

	if(pthread_mutex_destroy(&variables_mutex) == 0)
	{
		printf("MUTEX SEMAPHORE REALEASED 1\n");
	}
	else
	{
		printf("PROBLEM ON MUTEX SEMAPHORE REALEASE 1\n");
	}
    //need to unlock cause was locked to be destroyed
	pthread_mutex_unlock(&handler_mutex);
	if(pthread_mutex_destroy(&handler_mutex) == 0)
	{
		printf("MUTEX SEMAPHORE REALEASED 2\n");
	}
	else
	{
		printf("PROBLEM ON MUTEX SEMAPHORE REALEASE 2\n");
	}

    //releases the semaphore
	if(sem_destroy(&sem_threads) == 0)
	{
		printf("SEMAPHORE REALEASED\n");
	}
	else
	{
		printf("ERROR ON SEMAPHORE REALEASE\n");
	}

    //destroy condition variable
	pthread_cond_destroy(&threads_cond_var);
	printf("CONDITION VARIABLE DESTROYED\n");
    /*
    if(pthread_cond_destroy(&threads_cond_var)==0){
    	printf("CONDITION VARIABLE DESTROYED\n");
    }else{
    	printf("PROBLEM ON CONDITION VARIABLE DESTROYING\n");
    }*/



    //clean shared memory
	shmctl(shared_memory_id,IPC_RMID,NULL);
	printf("SHARED MEMORY REMOVED\n");
    //printf("a--- %d",shmctl(shared_memory_id,IPC_RMID,NULL));
    /*if(shmctl(shared_memory_id,IPC_RMID,NULL)==0){
    	printf("SHARED MEMORY REMOVED\n");
    }else{
    	printf("PROBLEM ON SHARED MEMORY REMOVING\n");
    }*/

    //free structs
	free(threads);
	printf("THREADS \"MEMORY\" REMOVED\n");
	free(Configurations);
	printf("CONFIGURATIONS MEMORY REMOVED\n");
	free(buffer_of_requests->list_requests);
	free(buffer_of_requests);
	printf("BUFFER_REQUESTS MEMORY REMOVED\n");
    //close socket
	if(close(socket_conn)==0)
	{
		printf("SOCKET CLOSED\n");
	}
	else
	{
		printf("PROBLEM ON SOCKET CLOSING\n");
	}

	while(wait(NULL) != -1)
	{
		printf("CHILD PROCESSE TERMINATED\n");
	}
	printf("SERVER TERMINATED\n");
	exit(0);
}
void create_shared_variable()
{
	if((shared_memory_id=shmget(IPC_PRIVATE,sizeof(struct stats),IPC_CREAT | 0777))<0)
	{
		perror("ERROR CREATING SHARED MEMORY FOR STATISTICS");
		exit(1);
	}
	printf("shared_memory_id : %d\n", shared_memory_id);
	if((Statistics = (Stats)shmat(shared_memory_id,NULL,0))==(Stats)-1)
	{
		perror("ERROR ATTACHING MEMORY");
		exit(1);
	}
}
void create_semaphores()
{
	sem_init(&sem_threads,0,1);
	sem_wait(&sem_threads);

    //sem_getvalue(&sem_threads, &value);
    //printf("valor %d\n",value);
}
