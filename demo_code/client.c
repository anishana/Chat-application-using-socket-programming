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
	char *port;
	char *IP;
	char *subString;
	while(TRUE){
		printf("\n[PA1-Client@CSE489/589]$ ");
		fflush(stdout);
		
		int receive=receive_msg_from_server(server);
		printf("Receive:%d\n",receive);
			
		char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
		memset(msg, '\0', MSG_SIZE);
		if(fgets(msg, MSG_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to msg
			exit(-1);
		//printf("argv:%s ",argv[2]);
		
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
			//strncpy(port_conf,port[0],strlen(port)-1);
			//printf("IP is %s and port is %s\n",IP, port);
			printf("%s %s %s %s \n",IP,port, argv[1],argv[2]);
			server = connect_to_host(IP, port);
			printf("\nConnection done. Return complete");
		}
		else if (strstr(msg,"SEND") && server != 0){
			printf("\nSENDing it to the remote server ... ");
			if(send(server, msg, strlen(msg), 0) == strlen(msg))
				printf("Done!\n");
			fflush(stdout);
			printf("\nEntered else if");
		} 
		else
		{
				
		}
		
	}
}

int receive_msg_from_server(int server)
{
	char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
	memset(buffer, '\0', BUFFER_SIZE);
	printf("In receive from server\n");
	
	if(recv(server, buffer, BUFFER_SIZE, 0) >= 0){
		printf("Server responded: %s", buffer);
		fflush(stdout);
		return 1;
	}

	return 0;
}

int connect_to_host(char *server_ip, char* server_port)
{
	int fdsocket;
	struct addrinfo hints, *res;

	/* Set up hints structure */	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

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
	
	printf("Connection done. Return");
	freeaddrinfo(res);

	return fdsocket;
}
