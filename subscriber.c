/* client header */
#include <stdio.h> 
#include <stdlib.h>     
#include <unistd.h>     
#include <string.h>     
#include <sys/types.h>     
#include <sys/socket.h>     
#include <arpa/inet.h>     
#include <netinet/in.h>
#include <inttypes.h>
#include <stdbool.h>
#include <fcntl.h>


int main(int argc, char const *argv[]) {
    /* check for nr args */
    if(argc != 4) {
        perror("you dont have right number of args");
        exit(EXIT_FAILURE);
    }
    
    /* set stdout BUFF */
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    /* parse args */
    char ID_CLIENT[20];
    strcpy(ID_CLIENT, argv[1]);
    char IP_SERVER[20];
    strcpy(IP_SERVER, argv[2]);
    char PORT_SERVER[20];
    strcpy(PORT_SERVER, argv[3]);
    /* get port */
    int PORT;
    sscanf(PORT_SERVER, "%" SCNd32 "", &PORT);
    if(PORT < 1024) {
        perror("incorrect port");
        exit(EXIT_FAILURE);
    }

    /* declare socket */
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_message[2000];
    
    /* Clean buffers and structures*/
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));
    memset(&server_addr, 0, sizeof(server_addr));
    
    /* Create socket, we use SOCK_STREAM for TCP */
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc < 0) {
        perror("[CLIENT] Unable to create socket\n");
        exit(EXIT_FAILURE);
    }
    
    /* Set port and IP the same as server-side */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(IP_SERVER);
    
    /* Send connection request to server */
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("[CLIENT] Unable to connect\n");
        exit(EXIT_FAILURE);
    }
    
    /* Send the message to server */
    if(send(socket_desc, ID_CLIENT, strlen(ID_CLIENT), 0) < 0){
        perror("[CLIENT] Unable to send ID_CLIENT\n");
        exit(EXIT_FAILURE);
    }
    
    /* Receive the response from server */
    if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
        printf("[CLIENT] Error while receiving server's msg\n");
        return -1;
    }
    printf("[CLIENT] Server's response: %s\n",server_message);

    /* make read from STDIN non-block */
    char command[20];
    int flag1 = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, flag1 | O_NONBLOCK);

    while(true) {
             
        /* wait for exit command */
        fgets(command, 20, stdin);
        if(command[0] != '\0' && strcmp(command, "exit\n") == 0) {
            break;
        }

        if(command[0] != '\0' && strcmp(command, "subscribe\n") == 0) {
            if(send(socket_desc, command, strlen(command), 0) < 0){
                perror("[CLIENT] Unable to send command\n");
                exit(EXIT_FAILURE);
            } else {
                printf("[CLIENT] Subscribe sent! -> %s", command);
            }
            memset(command, 0, sizeof(command));
        }

        if(command[0] != '\0' && strcmp(command, "unsubscribe\n") == 0) {
            if(send(socket_desc, command, strlen(command), 0) < 0){
                perror("[CLIENT] Unable to send command\n");
                exit(EXIT_FAILURE);
            } else {
                printf("[CLIENT] Unsubscribe sent! -> %s", command);
            }
            memset(command, 0, sizeof(command));
        }


    }
        
    /* Close the socket */
    close(socket_desc);
    
    return 0;
}