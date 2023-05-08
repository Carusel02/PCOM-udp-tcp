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
#include <poll.h>

struct pollfd fds[100];
int nr_fd = 0;

typedef struct block {
    
    int instruction;
    int size;
    int encode_type;

} block;

typedef struct encode0 {

    /* all have topics */
    char topic[60];
    unsigned int type;
    uint32_t INT;

    
    /* client UDP info */
    char ip_address[16];
    char port[6];

} encode0;

typedef struct encode1 {

    /* all have topics */
    char topic[60];
    unsigned int type;
    float SHORT_REAL;
    
    
    /* client UDP info */
    char ip_address[16];
    char port[6];


} encode1;

typedef struct encode2 {

    /* all have topics */
    char topic[60];
    unsigned int type;
    float FLOAT;

    /* client UDP info */
    char ip_address[16];
    char port[6];


} encode2;

typedef struct encode3 {

    /* all have topics */
    char topic[60];
    unsigned int type;

    /* client UDP info */
    char ip_address[16];
    char port[6];

    /* modify this */
    char payload[1600];

} encode3;

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

    int code;
    
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
    if(send(socket_desc, ID_CLIENT, sizeof(ID_CLIENT), 0) < 0){
        perror("[CLIENT] Unable to send ID_CLIENT\n");
        exit(EXIT_FAILURE);
    }
    
    /* Receive the response from server */
    if(recv(socket_desc, &code, sizeof(code), 0) < 0){
        perror("[CLIENT] Error while receiving server's msg\n");
        exit(EXIT_FAILURE);
    }
    printf("[CLIENT] Server's response: %d\n", code);

    if(code == 0) {
        printf("Disconnect\n");
        return 0;
    }

    /* make read from STDIN non-block */
    char command[100];

    /* set input from STDIN */
    fds[nr_fd].fd = STDIN_FILENO;
    fds[nr_fd].events = POLLIN;
    nr_fd++;

    /* add socket TCP */
    fds[nr_fd].fd = socket_desc;
    fds[nr_fd].events = POLLIN;
    nr_fd++;

    while(true) {

        /* make a poll, wait for readiness notification */
        int rv = poll(fds, nr_fd, -1);
        if (rv < 0) {
            perror("poll");
            exit(1);
        }

        /* read from STDIN */
        if ((fds[0].revents & POLLIN) != 0) {
            
            
            memset(command, 0, sizeof(command));
            fgets(command, 100, stdin);

            /* exit case */
            if(command[0] != '\0' && strcmp(command, "exit\n") == 0) {
                if(send(socket_desc, command, strlen(command), 0) < 0){
                    perror("[CLIENT] Unable to send command\n");
                    exit(EXIT_FAILURE);
                } else {
                    printf("[CLIENT] Subscribe sent! -> %s", command);
                }
                break;
            }
            
            /* subscribe case */
            if(command[0] != '\0' && strncmp(command, "subscribe ", 10) == 0) {
                if(send(socket_desc, command, strlen(command), 0) < 0){
                    perror("[CLIENT] Unable to send command\n");
                    exit(EXIT_FAILURE);
                } else {
                    printf("[CLIENT] Subscribe sent! -> %s", command);
                    printf("Subscribed to topic.\n");
                }
            }

            /* unsubscribe case */
            if(command[0] != '\0' && strncmp(command, "unsubscribe ", 12) == 0) {
                if(send(socket_desc, command, strlen(command), 0) < 0){
                    perror("[CLIENT] Unable to send command\n");
                    exit(EXIT_FAILURE);
                } else {
                    printf("[CLIENT] Unsubscribe sent! -> %s", command);
                    printf("Unsubscribed from topic.\n");
                }
            }
        }

        /* read from server SOCKET */
        if ((fds[1].revents & POLLIN) != 0) {
            /* Receive the response from server */
            block bloc;

            if(recv(socket_desc, &bloc, sizeof(block), 0) < 0){
                perror("[CLIENT] Error while receiving server's msg\n");
                exit(EXIT_FAILURE);
            } else {
                printf("Received from server: %d\n", bloc.instruction);
                
                switch (bloc.instruction) {
                /* disconnect client */
                case -1:
                    goto disconnect;
                break;

                /* receive a send */
                case 1: {
                    /* receive data */
                    
                    /* encode0 */
                    if(bloc.encode_type == 0) {
                        
                        encode0 encode;
                        memset(&encode, 0, sizeof(encode0));

                        if(recv(socket_desc, &encode, bloc.size, 0) < 0){
                            perror("[CLIENT] Error while receiving encode's 0 msg\n");
                            exit(EXIT_FAILURE);
                        } else {
                            printf("%s:%s - %s - INT - %d\n", encode.ip_address, encode.port, encode.topic, encode.INT);
                        }

                    }

                    /* encode1 */
                    if(bloc.encode_type == 1) {
                        
                        encode1 encode;
                        memset(&encode, 0, sizeof(encode1));

                        if(recv(socket_desc, &encode, bloc.size, 0) < 0){
                            perror("[CLIENT] Error while receiving encode's 1 msg\n");
                            exit(EXIT_FAILURE);
                        } else {
                            printf("%s:%s - %s - SHORT_REAL - %.2f\n", encode.ip_address, encode.port, encode.topic, encode.SHORT_REAL);
                        }

                    }

                    /* encode2 */
                    if(bloc.encode_type == 2) {
                        
                        encode2 encode;
                        memset(&encode, 0, sizeof(encode2));

                        if(recv(socket_desc, &encode, bloc.size, 0) < 0){
                            perror("[CLIENT] Error while receiving encode's 2 msg\n");
                            exit(EXIT_FAILURE);
                        } else {
                            printf("%s:%s - %s - FLOAT - %.2f\n", encode.ip_address, encode.port, encode.topic, encode.FLOAT);
                        }

                    }

                    /* encode3 */
                    if(bloc.encode_type == 3) {
                        
                        encode3 encode;
                        memset(&encode, 0, sizeof(encode3));

                        if(recv(socket_desc, &encode, bloc.size, 0) < 0){
                            perror("[CLIENT] Error while receiving encode's msg\n");
                            exit(EXIT_FAILURE);
                        } else {
                            printf("%s:%s - %s - STRING - %s\n", encode.ip_address, encode.port, encode.topic, encode.payload);
                        }

                    }



                } break;
                
                default:
                    perror("[CLIENT] receive error on bloc instruction");
                    exit(EXIT_FAILURE);
                    break;
                }


            }
        }

    }

    disconnect:   
    /* Close the socket */
    printf("[CLIENT] exit\n");
    close(socket_desc);
    
    return 0;
}