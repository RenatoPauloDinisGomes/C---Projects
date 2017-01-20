//-------------------STATS FUNCTIONS---------------------------//
int number_static_pages,
number_compressed_files,
num;
double average_time_content,
average_time_compressed_content;
//MMF Stuff
int id=0;

struct stat sbuf;
char *data;

int offset=0;
int PageSize=0;

void create_shared_variable()
{
	if((shared_memory_id=shmget(IPC_PRIVATE,sizeof(struct stats),IPC_CREAT | 0777))<0)
	{
		perror("ERROR CREATING SHARED MEMORY FOR STATISTICS");
		exit(1);
	}
	printf("shared_memory_id : %d size %ld \n ", shared_memory_id,sizeof(struct stats));
}


void server_stats()
{
	printf("Initialized server stats\n" );
	number_static_pages=0;
	number_compressed_files=0;
	average_time_content=0;
	average_time_compressed_content=0;
	num=0;

	signal(SIGTSTP ,run_stats);

	signal(SIGINT, close_stats);
	signal(SIGUSR1, print_stats);
	signal(SIGUSR2, stats_reset);

	while(1){pause();}
}
void stats_reset()
{
	number_static_pages=0;
	number_compressed_files=0;
	average_time_content=0;
	average_time_compressed_content=0;
	num=0;
	printf("\nStatistics Reset Completed\n");
}
void close_stats()
{
	time_t t_m;
	time(&t_m);
	print_stats();
	printf("\nStatistics Close time %s\n", ctime(&t_m));
	if (shmctl(shared_memory_id, IPC_RMID, NULL) < 0){
		perror( "Error deleting memory : " );
		exit(-1);	
	}
	exit(0);
}

void run_stats(){
	sem_wait(&mutex_stats);
	int fd;
	char temp[SIZE_BUF];
	offset=0;
	PageSize=0;
	if ((fd = open(STATS_FILE_NAME,O_RDWR | O_CREAT,0600)) < 0){
		perror("Error opening file for writing");
		exit(EXIT_FAILURE);
	}

	if((Statistics = (Stats)shmat(shared_memory_id,NULL,0))==(Stats)-1)
	{
		perror("ERROR ATTACHING MEMORY");
		exit(1);
	}

	switch(Statistics->request_type)
	{
		case 2:
		number_compressed_files++;
		average_time_compressed_content+=Statistics->duration;
		break;
		default:
		number_static_pages++;
		average_time_content+= Statistics->duration;
	}
	printf("\n--------------------------------\n");
	printf("Statistics entry: %d\n",++num);
	printf("File name: %s\n",Statistics->file_name);
	printf("Request Type (extension):%s\n",Statistics->request_type == 2 ? " .gz" : " .html");
	printf("Duration %lf seconds\n", Statistics->duration);
	printf("--------------------------------\n");

	PageSize= lseek(fd,0L,SEEK_END);
	strtok(Statistics->handled,"\n");
	sprintf(temp,"%d - Handled time: %s, Duration: %lf, File_Name: %s, Request Type (extension): %s, Arrival time: %s" ,id,Statistics->handled,Statistics->duration,Statistics->file_name,Statistics->request_type == 2 ? " .gz" : " .html",Statistics->arrival);
	PageSize+=strlen(temp);

	//PageSize+=sizeof(id)+sizeof(" - Handled time: , Duration: , File_Name: , Request Type (extension): , Arrival time: ")+strlen(strtok(Statistics->handled,"\n"))+sizeof(Statistics->duration)+strlen(Statistics->file_name)+strlen(Statistics->request_type == 2 ? " .gz" : " .html")+strlen(Statistics->arrival)-4;

	lseek(fd, PageSize-1, SEEK_SET);
	
    // put something there
	write(fd, " " , 1);
	//printf("Mapping file\n");

	if ((data = mmap(0,PageSize,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0)) == MAP_FAILED){
		perror("Error in mmap");
		exit(EXIT_FAILURE);
	}

	offset += strlen(data);
	id++;
	sprintf(data + offset,"%d - Handled time: %s, Duration: %lf, File_Name: %s, Request Type (extension): %s, Arrival time: %s" ,id,Statistics->handled,Statistics->duration,Statistics->file_name,Statistics->request_type == 2 ? " .gz" : " .html",Statistics->arrival);

	if(munmap(data,PageSize) < 0)
	{
		fprintf(stderr,"dst munmap error: %s\n",strerror(errno));
		exit(1);
	}

	shmdt(Statistics);
	close(fd);
	sem_post(&mutex_stats);
}

void print_stats()
{
	printf("--------Statistics--------\nNumber Compressed %d\n", number_compressed_files);
	printf("Number Static %d\n", number_static_pages);
	number_static_pages==0? printf("0 Static \n"):printf("Average_time_content %lf seconds\n",average_time_content/number_static_pages);
	number_compressed_files==0? printf("0 Compressed \n"):printf("Average_time_compressed_content %lf seconds\n",average_time_compressed_content/number_compressed_files);
}
