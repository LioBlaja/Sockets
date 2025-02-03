#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <ctype.h>
#include <string.h>

#define THREAD_COUNT 3


pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;

char string[1024];
int length = 0;

// 1. Să se scrie un program C ce implementează un client TCP. Programul va primi în linie de comandă adresa ip și portul serverului la care clientul se va conecta:
// ./prog <ip> <port>

// Programul va avea următoarea funcționalitate:
    // Thread-ul principal al programului se va conecta la server. Dacă conexiunea nu reușește programul se va termina. În cazul în care conexiunea reușește programul va creea 3 alte thread-uri noi care vor comunica printr-un buffer comun (string) și o variabilă comună (length) ce reprezintă dimensiunea datelor din buffer. Buffer-ul va conține doar un string la un moment dat. Thread-urile care vor scrie în buffer-ul string vor seta dimensiunea acestuia în variabila length.
    // Thread-ul 1 (write_to_socket): va monitoriza un buffer-ul și dimensiunea acestuia. În momentul în care variabila length va fi nenulă acest thread va citit buffer-ul comun, îl va trimite pe socket și după aceasta va reseta la zero variabila length.
    // Thread-ul 2 (read_from_stdin): va citi câte o linie de la intrarea standard și o va trimite către thread-ul 1, prin variabilele buffer și length pentru a fi trimisă pe socket. Acesta va copia linia în buffer-ul string și va seta length cu dimensiunea acestuia spre a fi preluată de thread-ul 1 pentru a fi transmisă pe socket
    // Thread-ul 3 (read_from_socket): va citi date text de la socket, va schimba literele mari în litere mici și invers și va trimite apoi rezultatul în bufferul string spre a fi apoi trimis de thread-ul 1 peste socket. Mecanismul de comunicare cu acesta este similar ca și la thread-ul 2.
// Se recomandă ca thread-urile să fie joinable. Este necesar ca variabilele string si length să fie controlate prin mutex.

// Programul va fi testat cu ajutorul unui server TCP ce va fi lansat cu ajutorul utilitarului netcat:

// nc -l <port> -s <ip> -v

pthread_cond_t conditionalVar;

void *thread1(void *arg) // write to socket
{

    int socketFd = *(int*)arg;

    while(1){
        pthread_mutex_lock(&my_mutex);
            while (length == 0)
            {
                pthread_cond_wait(&conditionalVar, &my_mutex);
            }

            // printf("STRING TH3:%s\n",string);
            // printf("Length TH3:%d\n",length);

            if(write(socketFd, string, length) < 0){
                perror("ERROR SENDING DATA TO THE SERVER");
                if(close(socketFd) < 0){
                    perror("ERROR CLOSING SOCKET");
                }
                exit(EXIT_FAILURE);
            }

            length = 0;
        pthread_mutex_unlock(&my_mutex);
    }


    return NULL;
}

void *thread2(void *arg){

    char temp[1024];

    while(fgets(temp,1024,stdin)){
        // printf("Read line:%s",temp);
        // printf("LEN:%ld\n",strlen(temp));
        int r = 0;

        if ((r = pthread_mutex_lock(&my_mutex)) != 0){
            fprintf(stderr, "Mutex lock error: %s\n", strerror(r));
            pthread_exit(NULL);
        }

        memset(string, 0, sizeof(string));
        
        length = strlen(temp) -1;
        strncpy(string,temp,length);

        pthread_cond_signal(&conditionalVar);

        // printf("BUFFER:%s\n",string);
        // printf("SHARED Len:%d\n",length);

        if ((r = pthread_mutex_unlock(&my_mutex)) != 0){
            fprintf(stderr, "Mutex unlock error: %s\n", strerror(r));
            pthread_exit(NULL);
        }
    }

    return NULL;
}

void *thread3(void *arg) // read from socket
{
    int socketFd = *(int*)arg;
    // printf("SOCKET FD:%d\n",socketFd);

    char temp[1024];
    int readLength = -1;

    while((readLength = read(socketFd,&temp,1024)) > 0){
        // printf("Read line:%s",temp);
        // printf("LEN:%ld\n",strlen(temp));
        int r = 0;

        if ((r = pthread_mutex_lock(&my_mutex)) != 0){
            fprintf(stderr, "Mutex lock error: %s\n", strerror(r));
            pthread_exit(NULL);
        }

        length = readLength - 1;//exclude \n
        // printf("THSD3:%d\n",length);

        // length = strlen(temp) - 1;

        for (int i = 0; i < length; i++)
        {
            temp[i] = toupper(temp[i]);
        }

        memset(string, 0, sizeof(string));
        strncpy(string,temp,length);
        pthread_cond_signal(&conditionalVar);

        // printf("BUFFER:%s\n",string);
        // printf("SHARED Len:%d\n",length);

        if ((r = pthread_mutex_unlock(&my_mutex)) != 0){
            fprintf(stderr, "Mutex unlock error: %s\n", strerror(r));
            pthread_exit(NULL);
        }
    }

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
    pthread_t threadHandlers[THREAD_COUNT];
    pthread_mutex_init(&my_mutex, NULL);
    int i = 0;

    if (pthread_create(&threadHandlers[0], NULL, thread1, (void *)&socketFd) != 0){
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&threadHandlers[1], NULL, thread2, NULL) != 0){
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&threadHandlers[2], NULL, thread3, (void *)&socketFd) != 0){
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threadHandlers[i], NULL);
    }

    // pthread_join(threadHandlers[1], NULL);
    // pthread_join(threadHandlers[2], NULL);


    close(socketFd);
    pthread_mutex_destroy(&my_mutex);

    return 0;
}
