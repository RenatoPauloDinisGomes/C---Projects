#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
// Produce debug information
#define DEBUG	  	1

// Header of HTTP reply to client
#define	SERVER_STRING 	"Server: simpleserver/0.1.0\r\n"
#define HEADER_1	"HTTP/1.0 200 OK\r\n"
#define HEADER_2	"Content-Type: text/html\r\n\r\n"

#define GET_EXPR	"GET /"
#define CGI_EXPR	"cgi-bin/"
#define SIZE_BUF	2048
#define SIZE_NAME	2048
#define SIZE_EXT 6
#define MAX_REQUESTS 100
#define MAX_FILES 10
#define PIPE_NAME "np_client_server"

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
    char str_1[1024];
} command;

typedef struct stats* Stats;
struct stats
{
    int number_static_pages, number_compressed_files, total_requests;
    float average_time_content, average_time_compressed_content;
};

typedef struct request* requests;

struct request
{
    time_t entry_time;
    /*struct tm {
       int tm_sec;          seconds,  range 0 to 59
       int tm_min;          minutes, range 0 to 59
       int tm_hour;         hours, range 0 to 23
       int tm_mday;         day of the month, range 1 to 31
       int tm_mon;          month, range 0 to 11
       int tm_year;         The number of years since 1900
       int tm_wday;         day of the week, range 0 to 6
       int tm_yday;         day in the year, range 0 to 365
       int tm_isdst;        daylight saving time
    };*/

    int  attended, socket, port, id, type;
    char file_name[SIZE_BUF],ip_str[INET6_ADDRSTRLEN],buf_aux[SIZE_BUF],buf_temp[SIZE_BUF];
};

typedef struct buffer * Buffer;
struct buffer
{
    int present_request,end_request,current_requests,end_temp_policy,temp_current_requests;
    requests list_requests;
};



/* Variables */

int port, socket_conn, new_conn, shared_memory_id, changing_policy=0;

//struct to save Configurations
Configs Configurations;
//struct to save Statistics
Stats Statistics;
//struct to sabe buffer request
Buffer buffer_of_requests;
//array to save threads
pthread_t *threads;
pthread_t scheduler_thread,pipe_thread;
//semaphoro for threads deprecated
sem_t sem_threads;
// variable of condition
pthread_mutex_t handler_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t variables_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t changing_policy_mutex = PTHREAD_MUTEX_INITIALIZER;
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
void catch_ctrlc(int);
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
void * worker();
void *scheduler_handler();
void reorganize_buffer();
int get_file_type(int index_in_buffer);

