/**
 * @server
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
 * This file contains the server init and main while loop for tha application.
 * Uses the select() API to multiplex between network I/O and STDIN.
 */
#include <stdio.h>

#include <stdlib.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <strings.h>

#include <string.h>

#include <unistd.h>

#include <sys/types.h>

#include <netdb.h>

#include "shell_commands.h"

#include <arpa/inet.h>

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256
#define MSG_SIZE 256

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
//int main(int argc, char **argv)
int run_server(int argc, char ** argv) {
  //while(TRUE){
  receive_msg(argc, argv);

  //}

  return 0;
}

struct client_details {
  int list_id;
  char hostname[50];
  char ip_addr[100];
  int fdaccept;
  int port_num;
};

struct blocked_details {
  int count;
  int fd_accept;
  char * blocked_ips_list[4]; // = {NULL, NULL, NULL, NULL};
};

void sendMessage(char * ip, char * message1, struct client_details client_list[100], struct blocked_details blocked_struct_list[5], int sock_index, char * buffer, int count_block_indexes);
void broadcast(char * message1, struct client_details client_list[100], struct blocked_details blocked_struct_list[5], int sock_index, char * buffer, int count_block_indexes);

void display(struct client_details client_list[100]);
void sort(struct client_details client_list[100]);
void adjust_list_ids(struct client_details client_list[100]);
void remove_from_list(struct client_details client_list[100], int key);

void receive_msg(int argc, char ** argv) {
  int server_socket, head_socket, selret, sock_index, caddr_len;
  int fdaccept;
  struct client_details client_list[100];
  struct blocked_details blocked_struct_list[5]; // = {NULL, NULL, NULL, NULL, NULL};	
  struct hostent * he;
  struct in_addr ipv4addr;
  struct sockaddr_in client_addr;
  struct addrinfo hints, * res;
  fd_set master_list, watch_list;
  int i = 0;
  int count_block_indexes = 0;
  int index_ip = -1;
  int j = 0;

  char message[100];

  /*for(i=0;i<4;i++)
  {
  	printf("Count %d\n", blocked_struct_list[i].count);
  
  }*/
  /* Set up hints structure */
  memset( & hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  /* Fill up address structures */
  if (getaddrinfo(NULL, argv[2], & hints, & res) != 0)
    perror("getaddrinfo failed");

  /* Socket */
  server_socket = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol);
  if (server_socket < 0)
    perror("Cannot create socket");

  if (bind(server_socket, res -> ai_addr, res -> ai_addrlen) < 0)
    perror("Bind failed");

  if (listen(server_socket, BACKLOG) < 0)
    perror("Unable to listen on port");

  FD_ZERO( & master_list);
  FD_ZERO( & watch_list);

  /* Register the listening socket */
  FD_SET(server_socket, & master_list);
  /* Register STDIN */
  FD_SET(STDIN, & master_list);

  head_socket = server_socket;

  while (TRUE) {
    memcpy( & watch_list, & master_list, sizeof(master_list));
    printf("[PA1-Server@CSE489/589]$ ");
    fflush(stdout);

    /* select() system call. This will BLOCK */
    selret = select(head_socket + 1, & watch_list, NULL, NULL, NULL);
    if (selret < 0)
      perror("select failed.");
    printf("selret:%d\n", selret);
    /* Check if we have sockets/STDIN to process */
    if (selret > 0) {

      /* Loop through socket descriptors to check which ones are ready */
      for (sock_index = 0; sock_index <= head_socket; sock_index += 1) {

        if (FD_ISSET(sock_index, & watch_list)) {

          /* Check if new command on STDIN */
          if (sock_index == STDIN) {

            //Process PA1 commands here ...
            char * msg = (char * ) malloc(sizeof(char) * MSG_SIZE);
            memset(msg, '\0', MSG_SIZE);
            if (fgets(msg, MSG_SIZE - 1, stdin) == NULL) //Mind the newline character that will be written to msg
              exit(-1);

            if (strcmp(msg, "IP\n") == 0) {
              getIp();
            } else if (strcmp(msg, "AUTHOR\n") == 0) {
              getAuthor();
            } else if (strcmp(msg, "PORT\n") == 0) {
              getPort(argv[2]);
            }

            free(msg);
          }
          /* Check if new client is requesting connection */
          else if (sock_index == server_socket) {
            caddr_len = sizeof(client_addr);
            fdaccept = accept(server_socket, (struct sockaddr * ) & client_addr, & caddr_len);
            if (fdaccept < 0)
              perror("Accept failed.");
            inet_pton(AF_INET, inet_ntoa(client_addr.sin_addr), & ipv4addr);
            he = gethostbyaddr( & ipv4addr, sizeof ipv4addr, AF_INET);

            printf("\n Remote Host connected!\n");
            /*printf("Host name: %s\n", he->h_name);
            printf("IP address is: %s\n", inet_ntoa(client_addr.sin_addr));   
            printf("port number %d\n", ntohs(client_addr.sin_port));
            printf("%d",client_list[0].list_id);*/
            for (i = 0; i < 100; i++) {
              if (client_list[i].list_id == 0)
                break;
            }

            printf("i:%d\n", i);
            if (client_list[i].list_id == 0) {
              client_list[i].list_id = 1;
              strcpy(client_list[i].hostname, he -> h_name);
              strcpy(client_list[i].ip_addr, inet_ntoa(client_addr.sin_addr));
              client_list[i].port_num = ntohs(client_addr.sin_port);
              client_list[i].fdaccept = fdaccept;
            } else {
              client_list[i].list_id = 1;
              strcpy(client_list[i].hostname, he -> h_name);
              strcpy(client_list[i].ip_addr, inet_ntoa(client_addr.sin_addr));
              client_list[i].port_num = ntohs(client_addr.sin_port);
              client_list[i].fdaccept = fdaccept;
            }
            //printf("client_list[i].ip_addr:%s\n",client_list[i].ip_addr);
            //printf("fdaccept:%d\n",fdaccept);
            sort(client_list);
            //printf("Done with sorting");

            adjust_list_ids(client_list);

            /* Add to watched socket list */
            FD_SET(fdaccept, & master_list);

            if (fdaccept > head_socket) head_socket = fdaccept;

          }
          /* Read from existing clients */
          else {
            /* Initialize buffer to receieve response */
            char * buffer = (char * ) malloc(sizeof(char) * BUFFER_SIZE);
            memset(buffer, '\0', BUFFER_SIZE);

            if (recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0) {
              close(sock_index);
              printf("Remote Host terminated connection!\n");

              /* Remove from watched list */
              FD_CLR(sock_index, & master_list);
            } else {
              //Process incoming data from existing clients here ...
              int receiver = 0;
              if (strstr(buffer, "LOGOUT")) {
                for (i = 0; i < 100; i++) {
                  if (client_list[i].fdaccept == sock_index)
                    break;

                }
                remove_from_list(client_list, client_list[i].fdaccept);
                sort(client_list);
                display(client_list);

              } else {
                //char *saveptr;
                printf("\nClient sent me: %s\n", buffer);

                char * fn_cp = malloc(strlen(buffer) + 1);
                strcpy(fn_cp, buffer);
                char * cmd = strtok(fn_cp, " ");

                //printf("ECHOing it back to the remote host ... ");

                if (strcmp(cmd, "BLOCK") == 0) {
                  char * block = malloc(strlen(buffer) + 1);
                  strcpy(block, buffer);
                  char * block_cmd = strtok(block, " ");
                  char * ip = strtok(NULL, "");

                  //for Block
                  printf("cmd:%s\n", cmd);
                  printf("ip:%s\n", ip);

                  int end_outer_loop = 0;
                  for (int k = 0; k < 5 && end_outer_loop == 0; k++) {

                    if ((count_block_indexes == k) || (blocked_struct_list[k].fd_accept != sock_index)) {
                      // if ip not current
                      if (count_block_indexes == k) {
                        // reached end, insert new ip
                        blocked_struct_list[k].fd_accept = sock_index;
                        blocked_struct_list[k].count++;
                        blocked_struct_list[k].blocked_ips_list[0] = malloc(strlen(ip) + 1);
                        strcpy(blocked_struct_list[k].blocked_ips_list[0], ip);
                        end_outer_loop = 1;
                        count_block_indexes++;
                        break;

                      } else if (blocked_struct_list[k].fd_accept != sock_index) {
                        // continue to next
                        continue;
                      }
                    } else {
                      // if ip is already in list, insert new blocked ip in list
                      for (j = 0; j < 4; j++) {
                        if ((blocked_struct_list[k].count == j) || (strcmp(blocked_struct_list[k].blocked_ips_list[j], ip) == 0))

                        {
                          if (blocked_struct_list[k].count == j) {
                            // reached end insert new ip
                            blocked_struct_list[k].blocked_ips_list[j] = malloc(strlen(ip) + 1);
                            strcpy(blocked_struct_list[k].blocked_ips_list[j], ip);
                            blocked_struct_list[k].count++;

                          } else if (strcmp(blocked_struct_list[k].blocked_ips_list[j], ip) == 0) {
                            // blocked ip already in list
                            printf("%d Already blocked %s ip\n", blocked_struct_list[k].fd_accept, ip);
                          }
                          end_outer_loop = 1;
                          break;

                        } else {
                          printf("Not end of list nor already blocked\n");
                          continue;

                        }

                      }

                    }
                  }

                } else if (strcmp(cmd, "SEND") == 0) {
                  // SEND message
                  char * send_msg = malloc(strlen(buffer) + 1);
                  strcpy(send_msg, buffer);
                  char * send_cmd = strtok(send_msg, " ");
                  char * ip = strtok(NULL, " ");
                  char * message1 = strtok(NULL, "");
                  sendMessage(ip, message1, client_list, blocked_struct_list, sock_index, buffer, count_block_indexes);

                } else if (strcmp(cmd, "BROADCAST") == 0) {
                  // BROADCAST message
                  char * message1 = strtok(NULL, "");

                  // printf("ip:%s\n", ip);
                  printf("message1:%s\n", message1);
                  broadcast(message1, client_list, blocked_struct_list, sock_index, buffer, count_block_indexes);

                }

              }
              free(buffer);
            }

          }
        }
      }
    }

  }
}

void sendMessage(char * ip, char * message1, struct client_details client_list[100], struct blocked_details blocked_struct_list[5], int sock_index, char * buffer, int count_block_indexes) {

  printf("ip:%s\n", ip);
  printf("message1:%s\n", message1);
  int receiver = 0;
  int do_not_send = 0;
  for (int i = 0; i < 100; i++) {

    if (strcmp(client_list[i].ip_addr, ip) == 0) {
      for (int k = 0; k < count_block_indexes; k++) {
        if (blocked_struct_list[k].fd_accept == sock_index) {
          for (int j = 0; j < blocked_struct_list[k].count; j++) {
            if (strcmp(blocked_struct_list[k].blocked_ips_list[j], ip) == 0) {
              do_not_send = 1;
              break;
            }
          }
        }
      }
      receiver = client_list[i].fdaccept;
      break;
    }
  }

  if (do_not_send == 0) {
    if (receiver > 0) {
      if (send(receiver, message1, strlen(buffer), 0) == strlen(buffer))
        printf("Done!\n");
      fflush(stdout);
    }
  } else {
    printf("%d has blocked ip %s. Hence message not sent", sock_index, ip);

  }
}

void broadcast(char * message1, struct client_details client_list[100], struct blocked_details blocked_struct_list[5], int sock_index, char * buffer, int count_block_indexes) {

  for (int i = 0; i < 100; i++) {

    if (client_list[i].list_id != 0 && sock_index != client_list[i].fdaccept && client_list[i].fdaccept!=0) {
      int do_not_send = 0;
      for (int k = 0; k < count_block_indexes; k++) {
        if (blocked_struct_list[k].fd_accept == sock_index) {
          for (int j = 0; j < blocked_struct_list[k].count; j++) {
            if (strcmp(blocked_struct_list[k].blocked_ips_list[j], client_list[i].ip_addr) == 0) {
              do_not_send = 1;
              break;
            }
          }

        }
      }
      
      if (do_not_send == 0) {
          if (send(client_list[i].fdaccept, message1, strlen(buffer), 0) == strlen(buffer))
            printf("Done!\n");
          fflush(stdout);
        } else {
          printf("%d has blocked ip %s. Hence message not sent", sock_index, client_list[i].ip_addr);
        }

    }
  }

}

void sort(struct client_details client_list[100]) {
  int i, j;
  struct client_details temp;

  for (i = 0; i <= 100; i++) {
    if (client_list[i + 1].port_num == 0) {
      break;
    }
    for (j = 0; j <= 100 - i; j++) { //printf("%d %d %d\n",i,j,client_list[j].port_num);
      if (client_list[j + 1].port_num == 0)
        break;

      if (client_list[j].port_num > client_list[j + 1].port_num) {

        temp = client_list[j];
        client_list[j] = client_list[j + 1];
        client_list[j + 1] = temp;
      }

    }

  }
}
void adjust_list_ids(struct client_details client_list[100]) {
  int i;

  for (i = 0; i < 100; i++) {
    if (client_list[i].list_id == 0)
      break;
    else
      client_list[i].list_id = i + 1;
  }
}

void remove_from_list(struct client_details client_list[100], int key) {
  int i, pointer;
  struct client_details temp;
  for (i = 0; i < 100; i++) {
    if (client_list[i].fdaccept == key)
      break;
  }
  client_list[i].list_id = 0;
  client_list[i].port_num = 0;

  for (pointer = i; pointer < 100; pointer++) {
    if (client_list[pointer].list_id == 0)
      break;
    else {
      temp = client_list[pointer];
      client_list[pointer] = client_list[pointer + 1];
      client_list[pointer + 1] = temp;
    }
  }

}

void display(struct client_details client_list[100]) {
  int i;
  for (i = 0; i < 100; i++) {
    if (client_list[i].list_id == 0)
      break;
    printf("%-5d%-35s%-20s%-8d\n", client_list[i].list_id, client_list[i].hostname, client_list[i].ip_addr, client_list[i].port_num);
  }
}
