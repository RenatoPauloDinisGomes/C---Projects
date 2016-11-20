
//-------------------STATS FUNCTIONS---------------------------//
int number_static_pages,
number_compressed_files,
num;
double average_time_content,
average_time_compressed_content;

void create_shared_variable()
{
	if((shared_memory_id=shmget(IPC_PRIVATE,sizeof(struct stats),IPC_CREAT | 0777))<0)
	{
		perror("ERROR CREATING SHARED MEMORY FOR STATISTICS");
		exit(1);
	}
	printf("shared_memory_id : %d\n", shared_memory_id);
}
void server_stats()
{
	FILE *fp;
	time_t t_m;
	fp = fopen(STATS_FILE_NAME,"a");

	printf("Initialized server stats\n" );
    //Statistics = (Stats)malloc(sizeof(struct stats));
	number_static_pages=0;
	number_compressed_files=0;
	average_time_content=0;
	average_time_compressed_content=0;
	num=0;

	signal(SIGINT, close_stats);
	signal(SIGUSR1, run_stats);
	signal(SIGUSR2, stats_reset);

	time(&t_m);
	fprintf(fp,"Start time %s\n", ctime(&t_m));
	fclose(fp);

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
	FILE *fp;
	time_t t_m;
	fp = fopen(STATS_FILE_NAME, "a");
	if((Statistics = (Stats)shmat(shared_memory_id,NULL,0))==(Stats)-1)
	{
		perror("ERROR ATTACHING MEMORY");
		exit(1);
	}
	time(&t_m);
	fprintf(fp, "----/-----/-----/-----/-----\n");
	fprintf(fp, "Close time %s", ctime(&t_m));
	fprintf(fp, "Number Compressed %d\n", number_compressed_files);
	fprintf(fp, "Number Static %d\n", number_static_pages);
	fprintf(fp, "----/-----/-----/-----/-----\n\n");

	print_stats();
	printf("\nStatistics Close time %s\n", ctime(&t_m));
	fclose(fp);
	exit(0);
}

void run_stats(){
	sem_wait(&mutex_stats);
	FILE *fp;

	fp = fopen(STATS_FILE_NAME, "a");
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

	fprintf(fp, "Handled time: %s, Duration: %lf, File_Name: %s, Request Type (extension): %s, Arrival time: %s\n",strtok(Statistics->handled,"\n"),Statistics->duration,Statistics->file_name,Statistics->request_type == 2 ? " .gz" : " .html",Statistics->arrival);
	shmdt(Statistics);
	fclose(fp);
	sem_post(&mutex_stats);
}

void print_stats()
{
	printf("\nNumber Compressed %d\n", number_compressed_files);
	printf("Number Static %d\n", number_static_pages);
	number_static_pages==0? printf("0 Static \n"):printf("Average_time_content %lf seconds\n",average_time_content/number_static_pages);
	number_compressed_files==0? printf("0 Compressed \n"):printf("Average_time_compressed_content %lf seconds\n",average_time_compressed_content/number_compressed_files);
}
