/**
 * @adsouza_assignment1
 * @author  Ashley Dsouza <adsouza@buffalo.edu>
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
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/logger.h"
#include <string.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256
/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
 
struct client_details
{
   int list_id; 
   char hostname[50];
   char ip_addr[100];
   int fdaccept;
   int port_num;
};

char address;
char str[INET_ADDRSTRLEN];
void display (struct client_details client_list[100]);
void sort (struct client_details client_list[100]);
void adjust_list_ids (struct client_details client_list[100]);
void remove_from_list (struct client_details client_list[100],char inet_addr [100]);


int main(int argc, char **argv)
{

	
	
	if (strcmp(argv[1],"c") == 1)
		printf("Cannot create client here");
	else{
	/*Init. Logger*/
	//cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	//fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
	if(argc != 2) {
		printf("Usage:%s [port]\n", argv[0]);
		exit(-1);
	}
	
	int server_socket, head_socket, selret, sock_index, fdaccept=0, caddr_len;
	struct sockaddr_in client_addr;
	struct addrinfo hints, *res;
	fd_set master_list, watch_list;
	struct hostent *he;
	struct in_addr ipv4addr;
	struct client_details client_list[100];
	int i;
	/* Set up hints structure */
	memset(&hints, 0, sizeof(hints));
    	hints.ai_family = AF_INET;
    	hints.ai_socktype = SOCK_STREAM;
    	hints.ai_flags = AI_PASSIVE;

	/* Fill up address structures */
	if (getaddrinfo(NULL, argv[1], &hints, &res) != 0)
		perror("getaddrinfo failed");
	
	/* Socket */
	server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(server_socket < 0)
		perror("Cannot create socket");
	
	/* Bind */
	if(bind(server_socket, res->ai_addr, res->ai_addrlen) < 0 )
		perror("Bind failed");

	freeaddrinfo(res);
	
	/* Listen */
	if(listen(server_socket, BACKLOG) < 0)
		perror("Unable to listen on port");
	
	/* ---------------------------------------------------------------------------- */
	
	/* Zero select FD sets */
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);
	
	/* Register the listening socket */
	FD_SET(server_socket, &master_list);
	/* Register STDIN */
	FD_SET(STDIN, &master_list);
	
	head_socket = server_socket;
	
	while(TRUE){
		memcpy(&watch_list, &master_list, sizeof(master_list));
		
		//printf("\n[PA1-Server@CSE489/589]$ ");
		//fflush(stdout);
		
		/* select() system call. This will BLOCK */
		selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
		if(selret < 0)
			perror("select failed.");
		
		/* Check if we have sockets/STDIN to process */
		if(selret > 0){
			/* Loop through socket descriptors to check which ones are ready */
			for(sock_index=0; sock_index<=head_socket; sock_index+=1){
				
				if(FD_ISSET(sock_index, &watch_list)){
					
					/* Check if new command on STDIN */
					if (sock_index == STDIN){
						char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);
						
						memset(cmd, '\0', CMD_SIZE);
						if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
							exit(-1);
						
						printf("\nI got: %s\n", cmd);
						
						//Process PA1 commands here ...
						
						free(cmd);
					}
					/* Check if new client is requesting connection */
					else if(sock_index == server_socket){
						caddr_len = sizeof(client_addr);
						
						
						fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
						if(fdaccept < 0)
							perror("Accept failed.");
						inet_pton(AF_INET, inet_ntoa(client_addr.sin_addr), &ipv4addr);
						he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
						printf("\n Remote Host connected!\n",str);
						/*printf("Host name: %s\n", he->h_name);
						printf("IP address is: %s\n", inet_ntoa(client_addr.sin_addr));   
						printf("port number %d\n", ntohs(client_addr.sin_port));
						printf("%d",client_list[0].list_id);*/
						for (i=0;i<100;i++){
							if (client_list[i].list_id == 0)
							break;
						}
						if (client_list[i].list_id  == 0)
						{client_list[i].list_id = 1;
							strcpy(client_list[i].hostname, he->h_name);
							strcpy(client_list[i].ip_addr ,inet_ntoa(client_addr.sin_addr));
							client_list[i].port_num = ntohs(client_addr.sin_port);
							client_list[i].fdaccept = fdaccept;
						}
						
						else {
							client_list[i].list_id = 1;
							strcpy(client_list[i].hostname,he->h_name);
							strcpy(client_list[i].ip_addr,inet_ntoa(client_addr.sin_addr));
							client_list[i].port_num = ntohs(client_addr.sin_port);
							client_list[i].fdaccept = fdaccept;
						}
						sort(client_list);
						//printf("Done with sorting");
						
						adjust_list_ids(client_list);
						display(client_list);
						
						/* Add to watched socket list */
						FD_SET(fdaccept, &master_list);
						if(fdaccept > head_socket) head_socket = fdaccept;
					}
					/* Read from existing clients */
					else{
						/* Initialize buffer to receieve response */
						char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
						memset(buffer, '\0', BUFFER_SIZE);
						
						if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
							remove_from_list(client_list,inet_ntoa(client_addr.sin_addr));
							close(sock_index);
							printf("%s Remote Host terminated connection!\n",inet_ntoa(client_addr.sin_addr));
							display(client_list);
							/* Remove from watched list */
							FD_CLR(sock_index, &master_list);
						}
						else {
							//Process incoming data from existing clients here ...
							
							printf("\nClient sent me: %s\n", buffer);
							printf("ECHOing it back to the remote host ... ");
							if(send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer))
								printf("Done!\n");
							fflush(stdout);
						}
						
						free(buffer);
					}
				}
			}
		}
	}
	
	return 0;
	}
}

void display (struct client_details client_list[100])
{
	int i;	
for (i=0;i<100;i++){
							if (client_list[i].list_id == 0)
								break;
							printf("%-5d%-35s%-20s%-8d\n", client_list[i].list_id, client_list[i].hostname, client_list[i].ip_addr, client_list[i].port_num);
						}
}

void sort (struct client_details client_list[100])
{
int i, j;
   struct client_details temp;
    
    for (i = 0; i <= 100; i++)
    {
		if (client_list[i+1].port_num==0) {
		break;}
        for (j = 0; j <= 100 - i; j++)
        {   //printf("%d %d %d\n",i,j,client_list[j].port_num);
			if (client_list[j+1].port_num==0)
			break;
				
			
            if (client_list[j].port_num > client_list[j + 1].port_num)
            {
				
                temp = client_list[j];
                client_list[j] = client_list[j + 1];
                client_list[j + 1] = temp;
            } 
			
			
			
		}
	
}
}
void adjust_list_ids (struct client_details client_list[100]){
	int i;
	
	for (i=0;i<100;i++)
	{
		if (client_list[i].list_id == 0)
			break;
		else
			client_list[i].list_id = i+1;
	}
}

void remove_from_list (struct client_details client_list[100],char inet_addr[100]){
	int i,pointer;
	struct client_details temp;
	for (i=0;i<100;i++)
	{
		if (strcmp(client_list[i].ip_addr,inet_addr) == 0)
		break;
	}
	client_list[i].list_id = 0;
	client_list[i].port_num = 0;

	for (pointer=i;pointer<100;pointer++)
	{
		if (client_list[pointer].list_id==0)
			break;
		else {
		temp = client_list[pointer];
                client_list[pointer] = client_list[pointer + 1];
                client_list[pointer + 1] = temp;
		}
	}
		
	
}