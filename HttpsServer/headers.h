#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>   
#include <sys/time.h>

#define __USE_GNU
#include <unistd.h>
#include <sys/mman.h> 

// Produce debug information
#define DEBUG	  	1

// Header of HTTP reply to client
#define	SERVER_STRING 	"Server: simpleserver/0.1.0\r\n"
#define HEADER_1	"HTTP/1.0 200 OK\r\n"
#define HEADER_2	"Content-Type: text/html\r\n\r\n"
#define GET_EXPR	"GET /"
#define CGI_EXPR	"cgi-bin/"
#define SIZE_BUF	1024
#define SIZE_NAME	1024
#define SIZE_EXT 6
#define MAX_REQUESTS 200
#define MAX_FILES 10
#define PIPE_NAME "np_client_server"
#define STATS_FILE_NAME "server.log"


/* structs */
typedef struct server_configs * Configs;
struct server_configs
{
  int server_port, thread_pool_size;
  char policy[SIZE_NAME];
  char allowed[MAX_FILES][SIZE_NAME];
};

typedef struct
{
  int cmd,thread_size,policy;
  char str_1[SIZE_BUF];
} command;

typedef struct stats* Stats;
struct stats
{
  int request_type;
  double duration;
  char file_name[SIZE_BUF],handled[SIZE_BUF],arrival[SIZE_BUF];
};

typedef struct request* requests;

struct request
{
  time_t entry_time;
  struct timeval tval_entry;
  int  attended, socket, port, id, type;
  char file_name[SIZE_BUF],ip_str[INET6_ADDRSTRLEN],buf_aux[SIZE_BUF],buf_temp[SIZE_BUF];
};

typedef struct buffer * Buffer;

struct buffer
{
  int present_request,end_request,current_requests,temp_current_requests;
  requests list_requests;
};

/* Variables */
int port, socket_conn,process_maker, new_conn, shared_memory_id, changing_policy=0,total_requests=0;

//struct to save Configurations
Configs Configurations;

//struct to save Statistics
Stats Statistics;

//struct to sabe buffer request
Buffer buffer_of_requests;

//array to save threads
pthread_t *threads;
pthread_t scheduler_thread, pipe_thread;

//semaphoro for threads deprecated
sem_t sem_threads;
sem_t mutex_stats;

// variable of condition
//pthread_mutex_t mutex_stats = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t handler_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t full = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t threads_cond_var = PTHREAD_COND_INITIALIZER;

/* functions */
int  fireup(int port);
void identify(int socket);
void get_request(int socket);
int  read_line(int socket, int n);
void send_header(int socket);
void send_page(int socket);
void execute_script(int socket);
void not_found(int socket);
void catch_ctrlc();
void catch_ctrlz();
void cannot_execute(int socket);
void Load_Configs();
void print_Configs();
void server_stats();
void start_server();
void create_thread_pool();
void terminate_threads();
void create_semaphores();
void create_shared_variable();
void create_shared_variable();
void handle_response(int index);
void *create_named_pipe();
void *worker();
void *scheduler_handler();
void reorganize_buffer();
int get_file_type(int index_in_buffer);
void close_stats();
void stats_reset();
void print_stats();
void run_stats();

