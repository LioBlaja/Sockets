#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/wait.h>

int main(int argc,char **argv){
    
    if(argc != 3){
        fprintf(stderr,"USAGE ./prog <ip> <port>");
        exit(EXIT_FAILURE);
    }

    int socketFd = -1;

    if((socketFd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in server_data;
    memset(&server_data, 0, sizeof(struct sockaddr_in));
    server_data.sin_family = AF_INET;
    server_data.sin_port = htons(atoi(argv[2]));
    server_data.sin_addr.s_addr = inet_addr(argv[1]);

    if(bind(socketFd,(struct sockaddr *)&server_data,sizeof(server_data)) < 0){
        perror("bind");
        close(socketFd);
        exit(EXIT_FAILURE);
    }

    if(listen(socketFd,3) < 0){
        perror("listen");
        close(socketFd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening for connection...\n");

    while(1){
        int client1Fd = -1;
        struct sockaddr_in client_1_data;

        int client_addr_len; // contine dimensiunea in bytes a adresei clientului scrisa in client_addr

        if((client1Fd = accept(socketFd,(struct sockaddr*)&client_1_data,&client_addr_len)) < 0){
            perror("accept");
            close(socketFd);
            exit(EXIT_FAILURE);
        }

        printf("Client connected\n");
        

        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(client1Fd);
            continue;
        } else if (pid == 0) {
            int readBytes = -1;
            char buffer[1024];
            
            while((readBytes = read(client1Fd,&buffer,1024)) > 0){
                for (int i = 0; i < readBytes - 1; i++)
                {
                    buffer[i] = toupper(buffer[i]);
                }

                if (write(client1Fd, buffer, readBytes - 1) < 0) {
                    perror("Error writing to client");
                    break;
                }    
            }
            exit(0);
        } else {
            // Procesul părinte
            
            close(client1Fd); // Părintele închide socket-ul clientului
        }
    }

    int status = -1;
    while(wait(&status) > 0){
        //continue
    }

    // close(client1Fd);
    close(socketFd);
    return 0;
}
