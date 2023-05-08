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
#include <poll.h>

#define MAXLINE 1600

int counter;

int key = 111;

typedef struct message {
    /* topic */
    char topic[60];
    /* type */
    unsigned int type;

    /* payload */
    char payload[1500];
    uint32_t INT;
    float SHORT_REAL;
    float FLOAT;

    /* client UDP info */
    char ip_address[16];
    char port[6];
    
    /* all clients that receive the message */
    int keys[100];

} message;

typedef struct block {




} block;

typedef struct database {
    /* client id */
    char client_id[20];
    /* file descriptor */
    struct pollfd *file_desc;
    /* index file descriptor */
    int index_file_desc;
    /* topics subscribed */
    char nr_topics_sub[100][60];
    /* topics number */
    int nr_topics;

    /* unique key */
    int key;
} database;

typedef struct client_state {
    /* clients disconnected */
    database disconnected[100];
    /* index clients disconnected */
    int index_disc;
} client_state;

struct pollfd fds[100];
int nr_fd = 0;

database base_data[100];
int nr_base_data = 0;

void extract_topic(char command[100], char topic[60]) {
    /* clear topic buffer */
    memset(topic, 0, 60);

    /* extract topic from here */
    char *token = strtok(command, " ");
    if(token != NULL)
        token = strtok(NULL, " ");

    token[strlen(token) - 1] = '\0';
    strcpy(topic, token);
}

int main(int argc, char const *argv[]) {   
    
    int code = 1;

    /* check nr topics */
    for(int i = 0 ; i < 100 ; i++) {
        base_data[i].nr_topics = 0;
    }

    /* server state */
    client_state state;
    state.index_disc = 0;


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
    if(listen(socket_tcp, 10) < 0) {
        perror("[TCP] Error while listening\n");
        exit(EXIT_FAILURE);
    }

    /* make read from STDIN non-block */
    char command[50];

    /* set input from STDIN */
    fds[nr_fd].fd = STDIN_FILENO;
    fds[nr_fd].events = POLLIN;
    nr_fd++;

    /* add socket UDP */
    fds[nr_fd].fd = sockfd_udp;
    fds[nr_fd].events = POLLIN;
    nr_fd++;

    /* add socket TCP */
    fds[nr_fd].fd = socket_tcp;
    fds[nr_fd].events = POLLIN;
    nr_fd++;


    /* create array messages */
    message mesaje[100];
    for(int i = 0 ; i < 100 ; i++) {
        for(int j = 0 ; j < 100 ; j++) {
            mesaje[i].keys[j] = -2;
        }
    }

     /* server loop */
    while (true) {

        checkpoint:
        /* make a poll, wait for readiness notification */
        int rv = poll(fds, nr_fd, -1);
        if (rv < 0) {
            perror("poll");
            exit(1);
        }

        /* iterate through clients */
        for (int i = 3; i < nr_fd; i++) {
            
            /* we received a command */
            if (fds[i].revents & POLLIN) {
                
                printf("Yahoo! We have something..\n");

                /* receive from tcp a command */
                char message_tcp[100];
                /* clear buffer */
                memset(message_tcp, 0, sizeof(message_tcp));

                ssize_t n = recv(fds[i].fd, message_tcp, sizeof(message_tcp), 0);
                if (n > 0) {
                    // process received data
                    // ...
                    if(message_tcp[0] != '\0' && strncmp(message_tcp, "subscribe ", 10) == 0) {
                        // process command

                        char topic[60];
                        extract_topic(message_tcp, topic);
                        printf("topic is : %s", topic);
                        strcpy(base_data[i - 3].nr_topics_sub[base_data[i - 3].nr_topics++], topic);
                        
                    }

                    if(message_tcp[0] != '\0' && strncmp(message_tcp, "unsubscribe ", 12) == 0) {
                        // process command
                        
                        char topic[60];
                        extract_topic(message_tcp, topic);
                        printf("topic is : %s\n", topic);


                        
                        printf("Client %s sent : %s", base_data[i - 3].client_id, message_tcp);
                    }

                    if(message_tcp[0] != '\0' && strcmp(message_tcp, "exit\n") == 0) {
                        // exit command

                        // client closed connection, close client socket
                        printf("Close socket from client %s\n", base_data[i - 3].client_id);
                        close(fds[i].fd);

                        /* clear fields */
                        memset(base_data[i - 3].client_id, 0, sizeof(base_data[i - 3].client_id));
                        memset(base_data[i - 3].nr_topics_sub, 0, sizeof(base_data[i - 3].nr_topics_sub));
                        // memset(&base_data[i].key, 0, sizeof(base_data[i].key)); // ??
                        base_data[i - 3].key = -1;
                    
                        /* clear index */
                        base_data[i - 3] = base_data[--nr_base_data];

                        /* for poll */
                        fds[i] = fds[--nr_fd];
                        i--;
                    }

                } else if (n == 0) {
                    // client closed connection, close client socket

                    /* save him in disconnected */
                    state.disconnected[state.index_disc++] = base_data[i - 3]; 
                   

                    printf("Close socket from client %s\n", base_data[i - 3].client_id);
                    close(fds[i].fd);

                    /* clear fields */
                    memset(base_data[i - 3].client_id, 0, sizeof(base_data[i - 3].client_id));
                    memset(base_data[i - 3].nr_topics_sub, 0, sizeof(base_data[i - 3].nr_topics_sub));
                    base_data[i - 3].key = -1;
                    
                    /* clear index */
                    base_data[i - 3] = base_data[--nr_base_data];


                    /* for poll */
                    fds[i] = fds[--nr_fd];
                    i--;
                    
                } else {
                    // error occurred, close client socket
                    perror("recv");
                    close(fds[i].fd);
                    

                    /* save him in disconnected */
                    state.disconnected[state.index_disc++] = base_data[i - 3]; 

                    /* clear fields */
                    memset(base_data[i - 3].client_id, 0, sizeof(base_data[i - 3].client_id));
                    memset(base_data[i - 3].nr_topics_sub, 0, sizeof(base_data[i - 3].nr_topics_sub));
                    base_data[i - 3].key = -1;

                    base_data[i - 3] = base_data[--nr_base_data];

                    /* for poll */
                    fds[i] = fds[--nr_fd];
                    i--;
                }
            }
        }


        /* read from STDIN */
        if ((fds[0].revents & POLLIN) != 0) {
            
            int terminator = 0;


            /* wait for exit command */
            memset(command, 0, sizeof(command));
            fgets(command, 50, stdin);
            if(command[0] != '\0' && strcmp(command, "exit\n") == 0) {
                terminator = 1;
                /* send to every client */
                for (int i = 3; i < nr_fd; i++) {
                    if(send(fds[i].fd, &terminator, sizeof(int), 0) < 0){
                        perror("[SERVER] Unable to send command\n");
                        exit(EXIT_FAILURE);
                    } else {
                        printf("[SERVER] TERMINATOR sent! -> %s", command);
                    }                
                }
                
                break;
            }

            // /* send hello command to all clients connected */
            // if(command[0] != '\0' && strcmp(command, "hello\n") == 0) {

            //     /* send to every client */
            //     for (int i = 0; i < nr_base_data; i++) {
            //         if(send(base_data[i].file_desc->fd, command, strlen(command), 0) < 0){
            //             perror("[SERVER] Unable to send command\n");
            //             exit(EXIT_FAILURE);
            //         } else {
            //             printf("[SERVER] HELLO sent! -> %s", command);
            //         }                
            //     }

            // }


        }

        /* read from UDP */
        if ((fds[1].revents & POLLIN) != 0) {
            
            /* clean buffer */
            memset(buffer, 0, sizeof(buffer));

            /* receive info from socket_udp */
            socklen_t len = sizeof(socket_udp);
            int n = recvfrom(sockfd_udp, (char *)buffer, MAXLINE,
                            MSG_WAITALL, (struct sockaddr *)&socket_udp,
                            &len);
            buffer[n] = '\0';

            /* if we received something from UDP client */
            if(n != -1) {

                /* extract data from buffer */
                message msg_udp;

                /* copy topic from BUFFER -> MSG_UDP.TOPIC */
                memcpy(msg_udp.topic, buffer, 50);
                /* make sure the 51th element is char '\0' for safety */
                msg_udp.topic[50] = '\0'; 
                /* copy type from BUFFER -> MSG_UDP.TYPE */
                memcpy(&msg_udp.type, buffer + 50, sizeof(uint8_t));
                
                /* complete payload */
                switch (msg_udp.type) {
                case 0: {
                    /* TYPE INT | CODE 0 | BYTE SIGN + INT_32 */
                    uint8_t sign;
                    uint32_t data;
                    /* copy sign from BUFFER */
                    memcpy(&sign, buffer + 51, sizeof(uint8_t));
                    /* copy data from BUFFER */
                    memcpy(&data, buffer + 52, sizeof(uint32_t));
                    /* make data from NETWORK -> HOST */
                    data = ntohl(data);

                    /* check sign */
                    if(sign == 1)
                        data -= 2 * data;
                    /* copy data from BUFFER -> MSG_UDP.INT */
                    msg_udp.INT = data;

                    printf("topic %s - tip date %d - payload %d\n", msg_udp.topic, msg_udp.type, msg_udp.INT);
                } break;

                case 1: {
                    /* TYPE SHORT_INT | CODE 1 | INT_16 */
                    uint16_t short_number;
                    memcpy(&short_number, buffer + 51, sizeof(uint16_t));
                    float result = htons(short_number) / 100.00;

                    msg_udp.SHORT_REAL = result;

                    printf("topic %s - tip date %d - payload %.3f\n", msg_udp.topic, msg_udp.type, msg_udp.SHORT_REAL);
                } break;

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
                } break;

                case 3: {
                    /* TYPE STRING | CODE 3 | STRING */
                    char payload[1500];        
                    // memcpy(msg_udp.payload, buffer + 51, (n - 50));
                    memcpy(payload, buffer + 51, (n - 50));
                    strncpy(msg_udp.payload, payload, (n-50));
                    
                    printf("topic %s - tip date %d - payload %s\n", msg_udp.topic, msg_udp.type, msg_udp.payload);
                } break;
                
                default:
                    perror("message UDP type failed");
                    exit(EXIT_FAILURE);
                    break;
                }

                /* complete client UDP info */
                strcpy(msg_udp.ip_address,inet_ntoa(socket_udp.sin_addr));
                sprintf(msg_udp.port,"%d",ntohs(client_addr.sin_port));

                /* add in vector messages */
                for(int kk = 0 ; kk < 100 ; kk++) {
                    msg_udp.keys[kk] = -1;
                }
                mesaje[counter++] = msg_udp;


            }

        }

        /* read from TCP */
        if ((fds[2].revents & POLLIN) != 0) {

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

            int already_con = 0;

            memset(client_id, 0, sizeof(client_id));
            client_size = sizeof(client_addr);
            
            /* Receive message from clients. Note that we use client_sock, not socket_tcp */
            if (recv(client_sock, client_id, sizeof(client_id), 0) < 0){
                perror("[TCP] Couldn't receive\n");
                exit(EXIT_FAILURE);
            }


            /* check id is already connected */
            for(int c = 0 ; c < nr_base_data ; c++) {
                // printf("client_id : %s base_data : %s\n", client_id, base_data[c].client_id);
                if(strcmp(client_id, base_data[c].client_id) == 0) {
                    printf("Client %s already connected.\n", client_id);
                    already_con = 1;
                    
                    /* send exit code to client */
                    code = 0;
                    if (send(client_sock, &code, sizeof(code), 0) < 0) {
                        perror("[TCP] Can't send while client code 0 \n");
                        exit(EXIT_FAILURE);
                    }

                    /* close connection */
                    close(client_sock);
                }
            }

            /* it s a new user */
            if(already_con == 0 ) {
                printf("New client %s connected from %s:%i.\n", client_id, 
                    inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                
                /* Write a response to the client */
                strcpy(server_message, "This is the server's message.");
                code = 1;


                if (send(client_sock, &code, sizeof(code), 0) < 0) {
                    perror("[TCP] Can't send while client code 1 \n");
                    exit(EXIT_FAILURE);
                }

                fds[nr_fd].fd = client_sock;
                fds[nr_fd].events = POLLIN;
                /* set client file descriptor index */
                base_data[nr_base_data].index_file_desc = nr_fd;


                /* clean buffer */
                memset(base_data[nr_base_data].client_id, 0, sizeof(base_data[nr_base_data].client_id));
                /* set client id */
                strcpy(base_data[nr_base_data].client_id, client_id);
                /* set client key */
                base_data[nr_base_data].key = key;
                key = key + 3;
                

                nr_base_data++;
                nr_fd++;




            }
            
        }

            goto checkpoint;
        }
        

        printf(" nr de conectari : %d\n", nr_base_data);

        int flag = 0;
        
        /* send to every client */
        for (int i = 3; i < nr_fd; i++) {
            /* check to see if it s a topic avaible */

            /* nr of all message topics */
            for(int m = 0 ; m < counter ; m++) {
                                

                /* nr of topics our client is subscribed */
                for(int total = 0 ; total < base_data[i - 3].nr_topics ; total++) {
                    
                    /* compare topics */
                    printf("\ncompare %s = %s acum backslash n \n", mesaje[m].topic, base_data[i - 3].nr_topics_sub[total]);

                    /* flag is 0 */
                    flag = 0;

                    /* we are on the same topic */
                    if(strcmp(mesaje[m].topic, base_data[i - 3].nr_topics_sub[total]) == 0) {
                        
                        /* iterate through all keys */
                        for(int nr_key = 0 ; nr_key < 10 ; nr_key++) {
                            
                            /* we send this message before to this client */
                            printf("\nmesaje key %d base_data %d key\n", mesaje[m].keys[nr_key], base_data[i - 3].key);
                            if(mesaje[m].keys[nr_key] == base_data[i - 3].key) {
                                /* go to the next message */
                                printf("\n WARN !! we send this before! \n");
                                flag = 1;
                                break;
                            }

                            /* if is not a key, put a key ! */
                            if(mesaje[m].keys[nr_key] == -1) {
                                printf("\n PUT A KEY HERE! \n");
                                mesaje[m].keys[nr_key] = base_data[i - 3].key;
                                break;
                            }
                        }
                        
                        /* go to the next message */
                        if(flag == 1)
                            continue;
                        
                        // send the topic
                        printf("ar trebui sa se trimita...\n");

                        if(send(fds[i].fd, mesaje[m].topic, sizeof(mesaje[m].topic), 0) < 0){
                            perror("[SERVER] Unable to send topic\n");
                            exit(EXIT_FAILURE);
                        } else {
                            printf("[SERVER] TOPIC sent! -> %s", command);

                        }  
                    }

                }

            }

            
                         
        }

        
    }


    for(int i = 0 ; i < counter ; i++) {
        printf("\n ******************************** \n");
        printf("    Message nr %d ip: %s port %s    \n", i + 1, mesaje[i].ip_address, mesaje[i].port);
        printf("Topic : %s\n", mesaje[i].topic);
        printf("Type : %d\n", mesaje[i].type);
        printf("Key : %d\n", mesaje[i].keys[0]);
        switch (mesaje[i].type)
        {
        case 0:
            printf("INT %d\n", mesaje[i].INT);
            break;
        case 1:
            printf("SHORT_REAL %f\n", mesaje[i].SHORT_REAL);
            break;
        case 2:
            printf("FLOAT %f\n", mesaje[i].FLOAT);
            break;
        case 3:
            printf("PAYLOAD %s\n", mesaje[i].payload);
            break;
        
        default:
            printf("Eroare stocare!\n");
            break;
        }

    }

    for(int i = 0 ; i < nr_base_data ; i++) {
        /* close the socket */
        printf("Client %s disconnected.\n", base_data[i].client_id);
        close(client_sock);
    }

    printf("socket_tcp disconnected\n");
    close(socket_tcp);
    printf("socket_udp disconnected\n");
    close(sockfd_udp);
    return 0;
}
