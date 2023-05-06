/* server header */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdbool.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>

#define MAXLINE 1500

int fd[100];
int nr_fd = 0;

int counter;

typedef struct message {
    char topic[50];
    int type;
    char payload[1500];
    char ip_address[16];
    char port[6];
    
    uint32_t INT;
    float SHORT_REAL;
    float FLOAT;
} message;

int main(int argc, char const *argv[]) {   
    
    /* check for nr args */
    if(argc != 2) {
        perror("you dont have right number of args");
        exit(EXIT_FAILURE);
    }

    /* get port */
    int PORT;
    sscanf(argv[1], "%"SCNd32"", &PORT);
    if(PORT < 1024) {
        perror("incorrect port");
        exit(EXIT_FAILURE);
    }
    
    /* set stdout BUFF */
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    /* ############# UDP SOCKET ############# */

    /* declare server socket */
    int sockfd_udp;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, socket_udp;

    /* create socket file descriptor */
    if ((sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[UDP] socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* set all fields with 0 */
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&socket_udp, 0, sizeof(socket_udp));

    /* complete SERVER information */
    servaddr.sin_family = AF_INET;         // ipv4
    servaddr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY = 0.0.0.0
    servaddr.sin_port = htons(PORT);       // port

    /* bind the socket with the server address */
    if (bind(sockfd_udp, (const struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0) {
        perror("[UDP] bind failed");
        exit(EXIT_FAILURE);
    }

    /* ############# TCP SOCKET ############# */

    int socket_tcp, client_sock;
    uint32_t client_size;
    struct sockaddr_in server_addr, client_addr;
    char server_message[2000], client_id[20];
    
    /* Clean buffers and structures*/
    memset(server_message, 0, sizeof(server_message));
    memset(client_id, 0, sizeof(client_id));
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    
    /* Create socket */
    socket_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_tcp < 0){
        perror("[TCP] Error while creating socket\n");
        exit(EXIT_FAILURE);
    }
    
    /* Set port and IP that we'll be listening for, any other IP_SRC or port will be dropped */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    /* Bind to the set port and IP */
    if(bind(socket_tcp, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[TCP] Couldn't bind to the port\n");
        exit(EXIT_FAILURE);
    }
    
    /* Listen for clients */
    if(listen(socket_tcp, 5) < 0) {
        perror("[TCP] Error while listening\n");
        exit(EXIT_FAILURE);
    }

    /* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
    
    // /*  Accept an incoming connection from one of the clients */
    // client_size = sizeof(client_addr);
    // client_sock = accept(socket_tcp, (struct sockaddr*)&client_addr, &client_size);
    // if (client_sock < 0){    
    //     perror("[TCP] Can't accept\n");
    //     exit(EXIT_FAILURE);
    // }
    
    // /* Receive message from clients. Note that we use client_sock, not socket_tcp */
    // if (recv(client_sock, client_id, sizeof(client_id), 0) < 0){
    //     perror("[TCP] Couldn't receive\n");
    //     exit(EXIT_FAILURE);
    // }

    // printf("New client %s connected from %s:%i.\n", client_id, 
    //     inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    // /* Write a response to the client */
    // strcpy(server_message, "This is the server's message.");
    
    // if (send(client_sock, server_message, strlen(server_message), 0) < 0) {
    //     perror("[TCP] Can't send\n");
    //     exit(EXIT_FAILURE);
    // }
    
    // /* Close the sockets */
    // printf("Client %s disconnected.\n", client_id);
    // close(client_sock);
    // close(socket_tcp);

    /* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

    /* make read from STDIN non-block */
    char command[10];
    int flag1 = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, flag1 | O_NONBLOCK);
    /* make server socket UDP non-block */
    int flag2 = fcntl(sockfd_udp, F_GETFL, 0);
    fcntl(sockfd_udp, F_SETFL, flag2 | O_NONBLOCK);
    /* make server socket TCP non-block */
    int flags3 = fcntl(socket_tcp, F_GETFL, 0);
    fcntl(socket_tcp, F_SETFL, flags3 | O_NONBLOCK);


    /* create array messages */
    message mesaje[50];

    while (true) {
        
        /* wait for exit command */
        fgets(command, 10, stdin);
        if(command[0] != '\0' && strcmp(command, "exit\n") == 0) {
            break;
        }

        /* receive info from socket_udp */
        socklen_t len = sizeof(socket_udp);
        int n = recvfrom(sockfd_udp, (char *)buffer, MAXLINE,
                         MSG_WAITALL, (struct sockaddr *)&socket_udp,
                         &len);
        buffer[n] = '\0';

        if(n != -1) {

        /* extract data from buffer */
        message msg_udp;
        memcpy(msg_udp.topic, buffer, 50);
        memcpy(&msg_udp.type, buffer + 50, sizeof(uint8_t));

        switch (msg_udp.type)
        {
        case 0: {
            /* TYPE INT | CODE 0 | BYTE SIGN + INT_32 */
            uint8_t sign;
            uint32_t data;
            memcpy(&sign, buffer + 51, sizeof(uint8_t));
            memcpy(&data, buffer + 52, sizeof(uint32_t));
            data = ntohl(data);
            // printf("sign : %"PRId8" data : %"PRId32"\n", sign, data);

            if(sign == 1)
                data -= 2 * data;

            msg_udp.INT = data;


            printf("topic %s - tip date %d - payload %d\n", msg_udp.topic, msg_udp.type, msg_udp.INT);
            mesaje[counter++] = msg_udp;
        }
            break;

        case 1: {
            /* TYPE SHORT_INT | CODE 1 | INT_16 */
            uint16_t short_number;
            memcpy(&short_number, buffer + 51, sizeof(uint16_t));
            float result = htons(short_number) / 100.00;

            msg_udp.SHORT_REAL = result;

            printf("topic %s - tip date %d - payload %.3f\n", msg_udp.topic, msg_udp.type, msg_udp.SHORT_REAL);
            mesaje[counter++] = msg_udp;
        }
            break;

        case 2: {
            /* TYPE FLOAT | CODE 2 | BYTE SIGN + INT_32 + EXPONENT */
            uint8_t sign;
            uint8_t exponent;
            uint32_t data = 0;
            memcpy(&sign, buffer + 51, sizeof(uint8_t));
            memcpy(&data, buffer + 52, sizeof(uint32_t));
            memcpy(&exponent, buffer + 56, sizeof(uint8_t));

            data = ntohl(data);
            float result = data / pow(10, exponent);

            if (sign == 1)
                result -= 2 * result;

            msg_udp.FLOAT = result;

            printf("topic %s - tip date %d - payload %.3f\n", msg_udp.topic, msg_udp.type, msg_udp.FLOAT);
            mesaje[counter++] = msg_udp;
        }
            break;

        case 3: {
            /* TYPE STRING | CODE 3 | STRING */
            char payload[1500];        
            // memcpy(msg_udp.payload, buffer + 51, (n - 50));
            memcpy(payload, buffer + 51, (n - 50));
            strncpy(msg_udp.payload, payload, (n-50));
            
            printf("topic %s - tip date %d - payload %s\n", msg_udp.topic, msg_udp.type, msg_udp.payload);
            mesaje[counter++] = msg_udp;
        }
            break;
        
        default:
            perror("message UDP type failed");
            exit(EXIT_FAILURE);
            break;
        }

        }

        /* receive info from socket_tcp */
        client_sock = accept(socket_tcp, (struct sockaddr*)&client_addr, &client_size);
        if (client_sock < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // no incoming connections, do something else or sleep
            } else {
                // error occurred
                if (client_sock < 0){    
                    perror("[TCP] Can't accept while client\n");
                    exit(EXIT_FAILURE);
                }
            }
        } else {
            // handle new connection on newfd
            // ...
            memset(client_id, 0, sizeof(client_id));
            client_size = sizeof(client_addr);
            
            /* Receive message from clients. Note that we use client_sock, not socket_tcp */
            if (recv(client_sock, client_id, sizeof(client_id), 0) < 0){
                perror("[TCP] Couldn't receive\n");
                exit(EXIT_FAILURE);
            }

            printf("New client %s connected from %s:%i.\n", client_id, 
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            
            /* Write a response to the client */
            strcpy(server_message, "This is the server's message.");
            
            if (send(client_sock, server_message, strlen(server_message), 0) < 0) {
                perror("[TCP] Can't send while client \n");
                exit(EXIT_FAILURE);
            }

            int flags4 = fcntl(client_sock, F_GETFL, 0);
            fcntl(client_sock, F_SETFL, flags4 | O_NONBLOCK);

            fd[nr_fd++] = client_sock;
            
            // /* close the socket */
            // printf("Client %s disconnected.\n", client_id);
            // close(client_sock);

        }

        /* receive from every fd */
        char message_tcp[20];
        memset(message_tcp, 0, sizeof(message_tcp));
        for(int i = 0 ; i < nr_fd ; i++) {
            if(recv(fd[i], message_tcp, sizeof(message_tcp), 0) < 0)
                continue;
            
            if(message_tcp[0] != '\0' && strcmp(message_tcp, "subscribe\n") == 0) {
                    printf("Client sent %s", message_tcp);
            }

            if(message_tcp[0] != '\0' && strcmp(message_tcp, "unsubscribe\n") == 0) {
                    printf("Client sent %s", message_tcp);
            }

        }
    }

    for(int i = 0 ; i < nr_fd ; i++) {
        /* close the socket */
        printf("Client %s disconnected.\n", client_id);
        close(client_sock);
    }
    close(socket_tcp);
    close(sockfd_udp);
    return 0;
}
