#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#define PIPE_NAME   "np_client_server"

typedef struct
{
    int cmd,thread_size,policy;
    char str_1[1024];
} command;

void print_menu();
int get_char();

int main()
{
    command cmd;
    char files[1024];
    int option,flag,option_2,thread_size;
    // Opens the pipe for writing
    int fd;
    if ((fd=open(PIPE_NAME, O_WRONLY)) < 0)
    {
        perror("Cannot open pipe for writing: ");
        exit(0);
    }
    do
    {
        cmd.cmd=0;
        cmd.thread_size=51;
        cmd.policy=0;
        flag ? printf("\e[1;1H\e[2J") : printf(" ");
        print_menu();
        scanf("%d",&option);
        flag=1;
        switch(option)
        {
            case 0:
            printf("Are you sure? (y/n) \n");
            if(get_char()){
                close(fd);
                exit(0);
            }
            else{
                printf("Tass bem\n");
                flag--;
            }
            break;
            case 1:
            printf("To what Policy do you whant to change\n1 - First in First out\n2 - Scheduling priority static content\n3 - Scheduling priority compressed static content\n");
            getchar();
            cmd.cmd=1;
            scanf("%d",&option_2);
            (option_2<4 && option_2>0) ? cmd.policy=option_2 : flag--;
            flag ? printf(" ") : printf("Invalid policy\n");
            break;
            case 2:
            cmd.cmd=2;
            printf("what is the new Thread pool size?\n");
            getchar();
            scanf("%d",&thread_size);
            (thread_size>0 && thread_size<=50) ?   cmd.thread_size=thread_size : flag--;
            flag ? printf(" ") : printf("Invalid size pool threads \n");
            break;
            case 3:
            cmd.cmd=3;
            printf("New allowed files? (ex. file_1.x;file_2.y;file_x.x)\n");
            getchar();
            fgets(cmd.str_1, sizeof(cmd.str_1), stdin);
            printf("files -> %s",cmd.str_1);
            break;
            default:
            printf("Invalid option\n");
            flag--;
        }
        flag ? write(fd, &cmd, sizeof(command)) : getchar();
    }
    while(1);

    return 0;
}
void print_menu()
{
    printf("------Admin Console------\n");
    printf("1 - Change policy\n");
    printf("2 - Change Threads pool size\n");
    printf("3 - Change allowed extensions\n");
    printf("0 - Exit\n");
}
int get_char()
{
    getchar();
    char sure;
    scanf("%c",&sure);
    if(sure=='y' || sure == 'Y')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
