/**
* @client
* @author  Ashley Sachin Anish
* @version 1.0
*
* @section LICENSE
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details at
* http://www.gnu.org/copyleft/gpl.html
*
* @section DESCRIPTION
*
* This file contains the client.
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include "client.h"
#include "shell_commands.h"
#include "../include/global.h"
#include "../include/logger.h"
#include <stdbool.h>

#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 256
#define BACKLOG 5
#define STDIN 0

//int connect_to_host(char *server_ip, char *server_port);

/**
* main function
*
* @param  argc Number of arguments
* @param  argv The argument list
* @return 0 EXIT_SUCCESS
*/
//int main(int argc, char **argv)
struct client_details
{
    int list_id;
    char hostname[50];
    char ip_addr[100];
    int fdaccept;
    int port_num;
};

void display_list(struct client_details client_list[100]);
void clear(struct client_details client_list[100]);
struct client_details client_list[100];
int list_ptr = 0;
char *status;
bool validateIpInList(char *ip);
bool validatePort(char *port);

int run_client(int argc, char **argv)
{
    int server = 0;
    int counter = 0;
    int fdaccept = 0, caddr_len;
    struct sockaddr_in server_addr;
    char *IP;
    char *port;
    char *subString;
    int head_socket, selret;
    fd_set master_list, watch_list;
    int client_socket = -1;
    struct sockaddr_in client;
    initialiseListsClient();
    /*for(int i=0; i<argc;i++)
        {
            printf("argv.%d:%s\n",i,argv[i]);
        }*/

    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /*
    if((client_socket = socket(AF_INET, SOCK_STREAM, 0))<0){
        perror("listen:Cannot create socket\n");
        exit(1);
    }
    printf("listen:Socket created\n");
    
    bzero((char *)&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    client.sin_port = htons(atoi(argv[2]));
    
    
    if((bind(client_socket,(struct sockaddr *)&client, sizeof(client)))<0){
        perror("Cannot bind\n");
        close(client_socket);
        exit(1);
    }
    printf("Bind done\n");
    if(listen(client_socket,BACKLOG)<0){
        perror("Listen failed\n");
        close(client_socket);
        exit(1);
    }
    printf("wait for connection to arrive\n");*/

    /* Register the listening socket */
    FD_SET(STDIN, &master_list);
    head_socket = server;

    printf("\n[PA1-Client@CSE489/589]$ ");
    while (TRUE)
    {
        fflush(stdout);

        FD_ZERO(&watch_list);
        memcpy(&watch_list, &master_list, sizeof(master_list));
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);

        if (selret < 0)
            perror("select failed.");

        /* Check if we have sockets/STDIN to process */
        if (selret > 0)
        {
            if (FD_ISSET(STDIN, &watch_list))
            {
                char *cmd = (char *)malloc(sizeof(char) * MSG_SIZE);
                memset(cmd, '\0', MSG_SIZE);
                if (fgets(cmd, MSG_SIZE - 1, stdin) == NULL) //Mind the newline character that will be written to msg
                    exit(-1);

                char *block = malloc(strlen(cmd) + 1);
                strcpy(block, cmd);
                char *msg = strtok(block, " ");

                if (strcmp(msg, "IP\n") == 0)
                {
                    getIp();
                }
                else if (strcmp(msg, "AUTHOR\n") == 0)
                {
                    getAuthor();
                }
                else if (strcmp(msg, "PORT\n") == 0)
                {
                    getPort(argv[2]);
                }
                else if (strcmp(msg, "LIST\n") == 0 && server != 0)
                {
                    display_list(client_list);
                }
                else if (strcmp(msg, "LOGIN") == 0 && server == 0)
                {
                    char buff[5], *buff2;

                    subString = strtok(cmd, " ");
                    IP = strtok(NULL, " ");

                    if (IP && validateIp(IP))
                    {
                        port = strtok(NULL, " ");
                        if (port && validatePort(port))
                        {
                            port[strlen(port) - 1] = 0;
                            server = connect_to_host(IP, port, argv[2]);
                            if (server > 0)
                            {
                                head_socket = (server > head_socket) ? server : head_socket;
                                FD_SET(server, &master_list);
                            }
                        }
                        else
                        {
                            errorMessage("LOGIN");
                            endMessage("LOGIN");
                        }
                    }
                    else
                    {
                        errorMessage("LOGIN");
                        endMessage("LOGIN");
                    }
                }
                else if (strcmp(msg, "REFRESH\n") == 0 && server != 0)
                {
                    list_ptr = 0;
                    char *saveptr;
                    clear(client_list);
                    // printf("\nSENDing Refresh to the remote server ... ");
                    if (send(server, cmd, strlen(cmd), 0) == strlen(cmd))
                    {
                        successMessage("REFRESH");
                        endMessage("REFRESH");
                        // printf("Done!\n");
                    }
                    fflush(stdout);
                }
                else if (strcmp(msg, "SEND") == 0 && server != 0)
                {
                    // exception handling needs to be done
                    char *saveptr;
                    // printf("\nSENDing it to the remote server ... \n");

                    cmd[strcspn(cmd, "\r\n")] = 0;

                    char *sendmsg = malloc(strlen(cmd) + 1);
                    strcpy(sendmsg, cmd);
                    char *ip;
                    if (strtok(sendmsg, " "))
                    {
                        ip = strtok(NULL, " ");
                    }

                    if (validateIp(ip) && validateIpInList(ip))
                    {
                        if (send(server, cmd, strlen(cmd), 0) == strlen(cmd))
                        {
                            // printf("Done!\n");
                            successMessage("SEND");
                            endMessage("SEND");
                        }
                    }
                    else
                    {
                        errorMessage("SEND");
                        endMessage("SEND");
                    }
                    fflush(stdout);
                }
                else if (strcmp(msg, "BROADCAST") == 0 && server != 0)
                {
                    // exception handling needs to be done
                    char *saveptr;
                    // printf("\nBROADCASTing it to the remote server ... ");
                    if (send(server, cmd, strlen(cmd), 0) == strlen(cmd))
                    {
                        // printf("Done!\n");
                        //successMessage("BROADCAST");
                        //endMessage("BROADCAST");
                    }
                    else
                    {
                        errorMessage("BROADCAST");
                        endMessage("BROADCAST");
                    }
                    fflush(stdout);
                }
                else if (strcmp(msg, "BLOCK") == 0 && server != 0)
                {
                    // exception handling needs to be done
                    char *saveptr;
                    // printf("\nBLOCKing IP ... ");

                    char *sendmsg = malloc(strlen(cmd) + 1);
                    strcpy(sendmsg, cmd);
                    char *ip;

                    if (strtok(sendmsg, " "))
                    {
                        ip = strtok(NULL, " ");
                    }

                    ip[strcspn(ip, "\r\n")] = 0;
                    if (validateIp(ip) && validateIpInList(ip))
                    {
                        send(server, cmd, strlen(cmd), 0);
                    }
                    else
                    {
                        errorMessage("BLOCK");
                        endMessage("BLOCK");
                    }

                    fflush(stdout);
                }
                else if (strcmp(msg, "UNBLOCK") == 0 && server != 0)
                {
                    // exception handling needs to be done
                    char *sendmsg = malloc(strlen(cmd) + 1);
                    strcpy(sendmsg, cmd);
                    char *ip;

                    if (strtok(sendmsg, " "))
                    {
                        ip = strtok(NULL, " ");
                    }

                    ip[strcspn(ip, "\r\n")] = 0;
                    if (validateIp(ip) && validateIpInList(ip))
                    {
                        send(server, cmd, strlen(cmd), 0);
                    }
                    else
                    {
                        errorMessage("UNBLOCK");
                        endMessage("UNBLOCK");
                    }
                    fflush(stdout);
                }
                else if (strcmp(msg, "LOGOUT\n") == 0 && server != 0)
                {
                    char *saveptr;
                    //printf("\nLOGOUTing it to the remote server ...\n");
                    char *logout_message = strtok(cmd, " ");
                    if (send(server, logout_message, strlen(logout_message), 0) == strlen(logout_message))
                    {
                        successMessage("LOGOUT");
                        list_ptr = 0;
                        clear(client_list);
                        close(server);
                        FD_CLR(server, &master_list);
                        server = 0;
                        endMessage("LOGOUT");
                    }
                    // else
                    // {
                    //     errorMessage("LOGOUT");
                    //     endMessage("LOGOUT");
                    // }
                    fflush(stdout);
                }
                else if (strcmp(msg, "EXIT\n") == 0)
                {
                    char *saveptr;
                    // printf("\nEXITing it to the remote server ...\n");
                    char *logout_message = strtok(cmd, " ");
                    if (send(server, logout_message, strlen(logout_message), 0) == strlen(logout_message))
                    {
                        list_ptr = 0;
                        clear(client_list);
                        close(server);
                        FD_CLR(server, &master_list);
                        server = 0;
                    }
                    else
                        printf("Not done\n");
                    fflush(stdout);
                    exit(0);
                }
                else if (server == 0)
                    printf("Client is not connected");
                FD_CLR(STDIN, &watch_list);
            }
            else if (FD_ISSET(server, &watch_list))
            {
                receive_msg_from_server(server);
                FD_CLR(server, &watch_list);
            }
        }
    }
}

int receive_msg_from_server(int server)
{
    char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
    memset(buffer, '\0', BUFFER_SIZE);

    if (recv(server, buffer, BUFFER_SIZE, 0) >= 0)
    {

        char *block = malloc(strlen(buffer) + 1);
        strcpy(block, buffer);
        char *msg = strtok(block, " ");

        // printf("msg:%s\n", msg);
        if (strcmp(msg, "LOGIN") == 0 || strcmp(msg, "REFRESH") == 0)
        {
            char *login = malloc(strlen(buffer) + 1);
            strcpy(login, msg);
            strtok(NULL, "");
            struct client_details *client = malloc(strlen(buffer) + 1);
            char *cmd;

            // printf("Received buffer: %s\n",buffer);
            sscanf(buffer, "%s %d %s %s %d", &cmd, &(client_list[list_ptr].list_id), &(client_list[list_ptr].hostname), &(client_list[list_ptr].ip_addr), &(client_list[list_ptr].port_num));
            // printf(" Done %d %s %s %d\n", client_list[list_ptr].list_id, client_list[list_ptr].hostname, client_list[list_ptr].ip_addr, client_list[list_ptr].port_num);
            // if (list_ptr == 0 && strcmp(msg, "LOGIN") == 0)
            // {
            //     successMessage("LOGIN");
            //     endMessage("LOGIN");
            // }

            char *nextClient = (char *)malloc(sizeof(char) * BUFFER_SIZE);
            list_ptr++;
            sprintf(nextClient, "%s %d", "GET_CLIENT", list_ptr);
            send(server, nextClient, strlen(nextClient) + 1, 0);
            if (strcmp(login, "LOGIN") == 0)
            {
                // printf("Only during login");
                send(server, "BUFFER 0", 8, 0);
            }
            // if (send(server, "BUFFER", 6, 0) == 1)
            // printf("Done!\n");
        }
        else if (strcmp(msg, "BUFFER") == 0)
        {
            // printf("buffer:%s\n", buffer);

            // char *cmd = (char *)malloc(sizeof(char) * BUFFER_SIZE);
            // char *fullmsg = (char *)malloc(sizeof(char) * BUFFER_SIZE);
            // char *receiverIp = (char *)malloc(sizeof(char) * BUFFER_SIZE);
            // char *IP = (char *)malloc(sizeof(char) * BUFFER_SIZE);
            // char *message = (char *)malloc(sizeof(char) * BUFFER_SIZE);
            // int index;
            //BUFFER 1 128.205.36.36 128.205.36.35 A
            // sscanf(buffer, "%s %d %s %s %s", &cmd, &index, &receiverIp, &IP, &message);
            char *cmd = strtok(buffer, " ");
            int index = atoi(strtok(NULL, " "));
            char *receiverIp = strtok(NULL, " ");
            char *IP = strtok(NULL, " ");
            char *message = strtok(NULL, "");

            // char *receiverIp = strtok(fullmsg, " ");
            // char *IP = strtok(NULL, " ");
            // char *message = strtok(NULL, "");

            successMessage("RECEIVED");
            cse4589_print_and_log("msg from:%s\n[msg]:%s\n", IP, message);
            endMessage("RECEIVED");

            char *buffer1 = (char *)malloc(sizeof(char) * BUFFER_SIZE);
            sprintf(buffer1, "%s %d", "BUFFER", index);

            send(server, buffer1, strlen(buffer1), 0);
        }
        else if (strcmp(msg, "NEXT_CLIENT") == 0)
        {
            // printf("NEXT_CLIENT Received buffer: %s\n",buffer);
            strtok(NULL, "");
            struct client_details *client = malloc(strlen(buffer) + 1);
            char *cmd;
            sscanf(buffer, "%s %d %s %s %d", &cmd, &(client_list[list_ptr].list_id), &(client_list[list_ptr].hostname), &(client_list[list_ptr].ip_addr), &(client_list[list_ptr].port_num));
            char *nextClient = (char *)malloc(sizeof(char) * BUFFER_SIZE);
            list_ptr++;
            sprintf(nextClient, "%s %d", "GET_CLIENT", list_ptr);
            // printf("NEXT_CLIENT nextClient: %s\n",nextClient);
            send(server, nextClient, strlen(nextClient) + 1, 0);
        }
        else if (strcmp(msg, "LOGINACK") == 0)
        {
            successMessage("LOGIN");
            endMessage("LOGIN");
        }
        else if (strcmp(msg, "BLOCKERROR") == 0)
        {
            errorMessage("BLOCK");
            endMessage("BLOCK");
        }
        else if (strcmp(msg, "BLOCKSUCCESS") == 0)
        {
            successMessage("BLOCK");
            endMessage("BLOCK");
        }
        else if (strcmp(msg, "UNBLOCKERROR") == 0)
        {
            errorMessage("UNBLOCK");
            endMessage("UNBLOCK");
        }
        else if (strcmp(msg, "UNBLOCKSUCCESS") == 0)
        {
            successMessage("UNBLOCK");
            endMessage("UNBLOCK");
        }
        else
        {
            char *IP = strtok(buffer, " ");
            char *message = strtok(NULL, "");

            successMessage("RECEIVED");
            cse4589_print_and_log("msg from:%s\n[msg]:%s\n", IP, message);
            endMessage("RECEIVED");

            fflush(stdout);
        }

        fflush(stdout);
        return 1;
    }
    else
        return 0;
}

int connect_to_host(char *server_ip, char *server_port, char *port)
{
    int fdsocket;
    struct addrinfo hints, *res;
    //struct sockaddr_in client;
    /* Set up hints structure */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    /* Fill up address structures */
    if (getaddrinfo(server_ip, server_port, &hints, &res) != 0)
    {
        errorMessage("LOGIN");
        endMessage("LOGIN");
    }
    /* Socket */
    fdsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fdsocket < 0)
    {
        errorMessage("LOGIN");
        endMessage("LOGIN");
        perror("Unable to create socket\n");
    }

    struct sockaddr_in my_addr1;
    my_addr1.sin_family = AF_INET;
    my_addr1.sin_addr.s_addr = INADDR_ANY;
    my_addr1.sin_port = htons(atoi(port));
    //my_addr1.sin_addr.s_addr = inet_addr("10.32.40.213");

    if (!bind(fdsocket, (struct sockaddr *)&my_addr1, sizeof(struct sockaddr_in)) == 0)
    {
        errorMessage("LOGIN");
        endMessage("LOGIN");
        perror("Unable to bind\n");
    }
    else
    {
        /* Connect */
        if (connect(fdsocket, res->ai_addr, res->ai_addrlen) < 0)
        {
            perror("Unable to connect\n");
            errorMessage("LOGIN");
        }
        else
            successMessage("LOGIN");
        endMessage("LOGIN");
        freeaddrinfo(res);
    }

    return fdsocket;
}
void display_list(struct client_details client_list[100])
{
    int i;
    successMessage("LIST");
    for (i = 0; i < 100; i++)
    {
        if (client_list[i].list_id == 0)

            break;
        cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", (i + 1), client_list[i].hostname, client_list[i].ip_addr, client_list[i].port_num);
    }
    endMessage("LIST");
}
void clear(struct client_details client_list[100])
{
    int i;
    for (i = 0; i < 100; i++)
    {
        if (client_list[i].list_id == 0)
            break;
        else
        {
            client_list[i].list_id = 0;
            client_list[i].port_num = 0;
        }
    }
}

bool validatePort(char *port)
{
    return ((atoi(port) > 1024) && (atoi(port) <= 65535));
}

bool validateIpInList(char *ip)
{
    for (int i = 0; i < 100; i++)
    {

        if (client_list[i].list_id == 0)
            break;
        // printf("client_list[i].ip_addr:%s\n",client_list[i].ip_addr);
        // printf("ip:%s\n",ip);

        if (client_list[i].ip_addr != NULL && ip != NULL && strcmp(client_list[i].ip_addr, ip) == 0)
        {
            return true;
        }
    }
    return false;
}
void initialiseListsClient()
{
    for (int i = 0; i < 100; i++)
    {
        client_list[i].list_id = 0;
        client_list[i].fdaccept = 0;
        client_list[i].port_num = 0;
    }
}