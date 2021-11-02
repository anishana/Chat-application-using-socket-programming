/**
* @client
* @author  Swetank Kumar Saha <swetankk@buffalo.edu>, Shivang Aggarwal <shivanga@buffalo.edu>
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
int run_client(int argc, char **argv)
{
	int server=0;
	int counter=0;
	int fdaccept=0,caddr_len;
	struct sockaddr_in server_addr;
	char *IP;
	char *port;
	char *subString;
	int head_socket,selret;
	fd_set master_list, watch_list;
	int client_socket=-1;
	struct sockaddr_in client;
	
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
	/* Register STDIN */
	FD_SET(server, &master_list);
	head_socket = server;
		
	while(TRUE){
		printf("[PA1-Client@CSE489/589]$ ");
		fflush(stdout);
		
		FD_ZERO(&watch_list);
		memcpy(&watch_list, &master_list, sizeof(master_list));
		selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
		
		if(selret < 0)
			perror("select failed.");
		
		//printf("selret:%d\n",selret);
		//printf("head_socket:%d\n",head_socket);
		/* Check if we have sockets/STDIN to process */
		if(selret > 0){
			if(FD_ISSET(STDIN, &watch_list)){
				char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
				memset(msg, '\0', MSG_SIZE);
				if(fgets(msg, MSG_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to msg
					exit(-1);
					
				if(strcmp(msg,"IP\n") == 0)
				{
					getIp();
				} 
				else if(strcmp(msg,"AUTHOR\n") == 0)
				{
					getAuthor();
				}
				else if(strcmp(msg,"PORT\n") == 0)
				{
					getPort(argv[2]);
				}
				else if (strstr(msg,"LOGIN") && counter == 0)
				{
				
					counter++;
					subString = strtok(msg," ");
					IP = strtok(NULL," ");
					port = strtok(NULL," ");
					port[strlen(port) - 1] =0;

					printf("%s %s %s %s \n",IP,port, argv[1],argv[2]);
					server = connect_to_host(IP, port,argv[2]);
					head_socket = (server > head_socket) ? server : head_socket;
					FD_SET(server, &master_list);
				}
				else if (strstr(msg,"SEND") && server != 0){
					char *saveptr;
					printf("\nSENDing it to the remote server ... ");
    					char *token=strtok(msg," ");
    					char *message1 = strtok(NULL,"");
    					
    					
					if(send(server,message1, strlen(message1), 0) == strlen(message1))
						printf("Done!\n");
					fflush(stdout);
				}

				else if (strstr(msg,"LOGOUT") ){
					char *saveptr;
					printf("\nSENDing it to the remote server ... ");
					char *logout_message =strtok(msg," ");
					if(send(server,logout_message, strlen(logout_message), 0) == strlen(logout_message))
						printf("Done!\n");
					else 
						printf("Not done\n");
					fflush(stdout);
					
    					
				}				
				FD_CLR(STDIN,&watch_list);
			
			} 
			else if (FD_ISSET(server, &watch_list))
			{
				receive_msg_from_server(server);
				FD_CLR(server,&watch_list);
			}
		}
		
	}
		
	
}

int receive_msg_from_server(int server)
{
	
	
	char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
	memset(buffer, '\0', BUFFER_SIZE);
	
	if(recv(server, buffer, BUFFER_SIZE, 0) >= 0){
		printf("Server responded: %s\n", buffer);
		fflush(stdout);
		return 1;
	} 
	else return 0;

}

int connect_to_host(char *server_ip, char* server_port, char *port)
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
		perror("getaddrinfo failed");

	/* Socket */
	fdsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(fdsocket < 0)
		perror("Failed to create socket");
	
	/* Connect */
	if(connect(fdsocket, res->ai_addr, res->ai_addrlen) < 0)
		perror("Connect failed");
	
    	/*
    	if(bind(fdsocket, (struct sockaddr *)&client, sizeof(sockaddr_in)) < 0 )
		perror("Bind failed");
	
	if(listen(fdsocket, BACKLOG) < 0)
		perror("Unable to listen on port");
		
		
	if((bind(fdsocket,(struct sockaddr *)&client, sizeof(client)))<0){
		perror("Cannot bind\n");
		close(fdsocket);
		exit(1);
	}
	printf("Bind done\n");

	if(listen(fdsocket,BACKLOG)<0){
		perror("Listen failed\n");
		close(fdsocket);
		exit(1);
	}
	printf("wait for connection to arrive\n");
	*/
		
		
	freeaddrinfo(res);

	return fdsocket;
}
