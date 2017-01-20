//-------------------.Threads Functions---------------------------------//
void clean_worker(){
	pthread_exit(NULL);
}
void * worker(){
	signal(SIGUSR2, clean_worker);
	while(1)
	{
		sem_wait(&sem_threads);
		changing_policy ? printf("Changing policy\n") : printf("\n");
		pthread_mutex_lock(&mutex_buffer);

		if(++buffer_of_requests->present_request == MAX_REQUESTS) buffer_of_requests->present_request=0;

		int index = buffer_of_requests->present_request;
		
		handle_response(index);
		pthread_cond_signal(&threads_cond_var);
		//sleep(1);
	}
	return 0;
}

void clean_scheduler_handler(){
	pthread_exit(NULL);
}
void *scheduler_handler(){
	signal(SIGUSR1, clean_scheduler_handler);
	int flag=1;
	while(1)
	{
		pthread_mutex_lock(&handler_mutex);
		//wait while there is no requests 
		while(buffer_of_requests->current_requests==0)
			pthread_cond_wait(&threads_cond_var,&handler_mutex);
		pthread_mutex_unlock(&handler_mutex);
	
		reorganize_buffer();
		if(buffer_of_requests->current_requests<=Configurations->thread_pool_size && !changing_policy){
			printf("Thread freed to attend request\n");
			buffer_of_requests->current_requests--;
			sem_post(&sem_threads);
		}
		else if(changing_policy){
			!flag ? printf(" "): printf("Please wait changing policy...\n") ;
			flag=0;
		}
		else{
			!flag ? printf(" "): printf("Full please wait ...\n");
			flag=0;

		}
	}
	return 0;
}

void clean_named_pipe(){
	pthread_exit(NULL);
}

void *create_named_pipe(){
	signal(SIGUSR1, clean_named_pipe);
	int i=0;
	char * pch;
		// Creates the named pipe if it doesn't exist yet
	if ((mkfifo(PIPE_NAME, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)){
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
			if(Configurations->thread_pool_size<cmd.thread_size){
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