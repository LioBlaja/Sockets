#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <ctype.h>
#include <sys/types.h>
#include <pthread.h>

typedef struct{
    int pipe;
    int socketFd;
}Data;

void *threadReadPipe3_1(void *arg) // write to socket
{
    Data *data = (Data*)arg;

    int pipe = data->pipe;   
    int socketFd = data->socketFd;
    // Data data = *(Data*)arg;

    char temp[1024];
    int readLength = -1;

    // printf("th1\n");

    while((readLength = read(pipe,&temp,1024)) > 0){
        // printf("PIPE_1_3:%s\n",temp);

        if(write(socketFd, temp, readLength) < 0){
            perror("ERROR SENDING DATA TO THE SERVER");
            if(close(socketFd) < 0){
                perror("ERROR CLOSING SOCKET");
            }
            exit(EXIT_FAILURE);
        }

        memset(temp,0,sizeof(temp));
    }

    free(data); 
    return NULL;
}

void *threadReadPipe1_2(void *arg) // write to socket
{
    // int pipe = *(int*)arg;
    Data *data = (Data*)arg;

    int pipe = data->pipe;   
    int socketFd = data->socketFd;
    // printf("J:%d\n",pipe);
    // printf("J:%d\n",socketFd);
    // printf("th2\n");
    char temp[1024];
    int readLength = -1;

    while((readLength = read(pipe,&temp,1024)) > 0){
        // printf("PIPE_1_2:%s\n",temp);

        if(write(socketFd, temp, readLength) < 0){
            perror("ERROR SENDING DATA TO THE SERVER");
            if(close(socketFd) < 0){
                perror("ERROR CLOSING SOCKET");
            }
            exit(EXIT_FAILURE);
        }
        memset(temp,0,sizeof(temp));
    }

    free(data); 
    return NULL;
}

int main(int argc,char** argv){
    
    if(argc != 3){
        fprintf(stderr,"USAGE ./prog <ip> <port>\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);
    // printf("PORT:%d\n",port);
    
    char* ipAddress = argv[1];
    // printf("Address:%s\n",ipAddress);

    int socketFd = -1;

    if((socketFd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("ERROR SOCKET CREATION");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in my_addr_struct;
    memset(&my_addr_struct, 0, sizeof(struct sockaddr_in));
    my_addr_struct.sin_family = AF_INET;
    my_addr_struct.sin_port = htons(port);
    my_addr_struct.sin_addr.s_addr = inet_addr(ipAddress);

    if(connect(socketFd,(struct sockaddr *)&my_addr_struct,sizeof(my_addr_struct)) < 0){
        perror("connect");
        close(socketFd);
        exit(EXIT_FAILURE);
    }

    printf("CONNECTION ESTABLISHED\n");

    // create 3 threads
    pid_t pid1,pid2,pid3;

    int pipe_1_2[2];
    // int pipe_2_3[2];
    int pipe_3_1[2];

    if(pipe(pipe_1_2) < 0){
        perror("Pipe");
        close(socketFd);
        exit(EXIT_FAILURE);
    }
    // if(pipe(pipe_2_3) < 0){
    //     perror("Pipe");
    //     close(socketFd);
    //     exit(EXIT_FAILURE);
    // }
    if(pipe(pipe_3_1) < 0){
        perror("Pipe");
        close(socketFd);
        exit(EXIT_FAILURE);
    }

    //proc1
    if((pid1=fork()) < 0){
        perror("Eroare");
        exit(1);
    }
    if(pid1==0){
        close(pipe_3_1[1]);//scriere
        close(pipe_1_2[1]);//scriere

        //add 2 threads to read from the pipes
        pthread_t threadHandlers[2];

        Data *data = malloc(sizeof(Data));
        if (!data) {
            perror("ERROR ALLOCATING MEMORY");
            exit(EXIT_FAILURE);
        }
        data->pipe = pipe_1_2[0];
        data->socketFd = socketFd;
        
        Data *data1 = malloc(sizeof(Data));
        if (!data1) {
            perror("ERROR ALLOCATING MEMORY");
            exit(EXIT_FAILURE);
        }
        data1->pipe = pipe_3_1[0];
        data1->socketFd = socketFd;

        if (pthread_create(&threadHandlers[0], NULL, threadReadPipe3_1, (void*)data1) != 0){
            close(pipe_3_1[0]);
            close(pipe_1_2[0]);
            close(socketFd);
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&threadHandlers[1], NULL, threadReadPipe1_2, (void*)data) != 0){
            close(pipe_3_1[0]);
            close(pipe_1_2[0]);
            close(socketFd);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < 2; i++)
        {
            pthread_join(threadHandlers[i], NULL);
        }

        close(pipe_3_1[0]);
        close(pipe_1_2[0]);
        exit(0); // apel necesar pentru a se opri codul fiului astfel incat acesta sa nu execute si codul parintelui
    }
    //proc2
    if((pid2=fork()) < 0){
        perror("Eroare");
        exit(1);
    }
    if(pid2==0){
        // close(pipe_3_1[0]);
        // close(pipe_3_1[1]);
        // close(pipe_1_2[0]);

        char temp[1024];

        while(fgets(temp,1024,stdin)){
            // printf("Read line:%s",temp);
            // printf("LEN:%ld\n",strlen(temp));
            if(write(pipe_1_2[1],temp,strlen(temp) - 1) < 0){
                perror("write to pipe");
                close(socketFd);
                close(pipe_1_2[1]);
                exit(EXIT_FAILURE);
            }
            memset(temp,0,sizeof(temp));
        }
        // }

        close(pipe_1_2[1]);
        exit(0); // apel necesar pentru a se opri codul fiului astfel incat acesta sa nu execute si codul parintelui
    }
    //proc3
    if((pid3=fork()) < 0){
        perror("Eroare");
        exit(1);
    }
    if(pid3==0){

        close(pipe_1_2[0]);
        close(pipe_1_2[1]);
        close(pipe_3_1[0]);

        char temp[1024];
        int readLength = -1;

        while((readLength = read(socketFd,&temp,1024)) > 0){
            for (int i = 0; i < (readLength - 1); i++)
            {
                temp[i] = toupper(temp[i]);
            }

            if(write(pipe_3_1[1],temp,readLength) < 0){
                perror("write to pipe");
                close(socketFd);
                close(pipe_3_1[1]);
                exit(EXIT_FAILURE);
            }
            memset(temp,0,sizeof(temp));
        }

        close(pipe_3_1[1]);
        exit(0); // apel necesar pentru a se opri codul fiului astfel incat acesta sa nu execute si codul parintelui
    }

    while (waitpid(-1, NULL, 0) > 0) {
        // Waiting for all child processes to terminate
    }


    close(socketFd);
    
    return 0;
}
