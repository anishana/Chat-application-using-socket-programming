/**
 *@server
 *@author  Ashley Sachin Anish
 *@version 1.0
 *
 *@section LICENSE
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License as
 *published by the Free Software Foundation; either version 2 of
 *the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful, but
 *WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *General Public License for more details at
 *http://www.gnu.org/copyleft/gpl.html
 *
 *@section DESCRIPTION
 *
 *This file contains the server init and main while loop for tha application.
 *Uses the select() API to multiplex between network I/O and STDIN.
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

#include "server.h"
#include "../include/global.h"
#include "../include/logger.h"
#include <stdbool.h>

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256
#define MSG_SIZE 256

/**
 *main function
 *
 *@param  argc Number of arguments
 *@param  argv The argument list
 *@return 0 EXIT_SUCCESS
 */
//int main(int argc, char **argv)
int run_server(int argc, char **argv)
{
  //while(TRUE){
  receive_msg(argc, argv);

  //}

  return 0;
}

void receive_msg(int argc, char **argv)
{

  int server_socket, head_socket, selret, sock_index, caddr_len;
  int fdaccept;
  struct client_details client_list[100];
  struct blocked_details blocked_struct_list[5]; // = {NULL, NULL, NULL, NULL, NULL};
  struct message_details message_buffer_list[5];
  struct hostent *he;
  struct in_addr ipv4addr;
  struct sockaddr_in client_addr;
  struct addrinfo hints, *res;
  fd_set master_list, watch_list;
  int i = 0;
  int count_block_indexes = 0;
  int index_ip = -1;
  int j = 0;
  int max_receiver_ips = 0;
  char message[100];

  /*Set up hints structure */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  /*Fill up address structures */
  if (getaddrinfo(NULL, argv[2], &hints, &res) != 0)
    perror("getaddrinfo failed");

  /*Socket */
  server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (server_socket < 0)
    perror("Cannot create socket");

  if (bind(server_socket, res->ai_addr, res->ai_addrlen) < 0)
  {
    perror("Bind failed");
    exit(-1);
  }

  if (listen(server_socket, BACKLOG) < 0)
    perror("Unable to listen on port");

  FD_ZERO(&master_list);
  FD_ZERO(&watch_list);

  /*Register the listening socket */
  FD_SET(server_socket, &master_list);
  /*Register STDIN */
  FD_SET(STDIN, &master_list);

  head_socket = server_socket;

  while (TRUE)
  {
    memcpy(&watch_list, &master_list, sizeof(master_list));
    printf("[PA1-Server@CSE489/589]$ ");
    fflush(stdout);

    /*select() system call. This will BLOCK */
    selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
    if (selret < 0)
      perror("select failed.");
    /*Check if we have sockets/STDIN to process */
    if (selret > 0)
    {

      /*Loop through socket descriptors to check which ones are ready */
      for (sock_index = 0; sock_index <= head_socket; sock_index += 1)
      {

        if (FD_ISSET(sock_index, &watch_list))
        {

          /*Check if new command on STDIN */
          if (sock_index == STDIN)
          {

            //Process PA1 commands here ...
            char *msg = (char *)malloc(sizeof(char) * MSG_SIZE);
            memset(msg, '\0', MSG_SIZE);
            if (fgets(msg, MSG_SIZE - 1, stdin) == NULL) //Mind the newline character that will be written to msg
              exit(-1);

            if (strcmp(msg, "IP\n") == 0)
            {
              getIp();
            }
            else if (strcmp(msg, "LIST\n") == 0)
            {
              display(client_list);
            }
            else if (strcmp(msg, "AUTHOR\n") == 0)
            {
              getAuthor();
            }
            else if (strcmp(msg, "PORT\n") == 0)
            {
              getPort(argv[2]);
            }
          }
          else if (sock_index == server_socket)
          {
            caddr_len = sizeof(client_addr);
            fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
            if (fdaccept < 0)
              perror("Accept failed.");
            inet_pton(AF_INET, inet_ntoa(client_addr.sin_addr), &ipv4addr);
            he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);

            printf("\n Remote Host connected!\n");
            for (i = 0; i < 100; i++)
            {
              if (client_list[i].list_id == 0)
                break;
            }

            client_list[i].list_id = 1;
            strcpy(client_list[i].hostname, he->h_name);
            strcpy(client_list[i].ip_addr, inet_ntoa(client_addr.sin_addr));
            client_list[i].port_num = ntohs(client_addr.sin_port);
            client_list[i].fdaccept = fdaccept;

            sort(client_list);

            adjust_list_ids(client_list);

            /* Add to watched socket list */
            FD_SET(fdaccept, &master_list);

            if (fdaccept > head_socket)
              head_socket = fdaccept;
            char *buffer1 = (char *)malloc(sizeof(char) * BUFFER_SIZE);
            char *logged_in_ip = client_list[i].ip_addr;

            int k = 0;
            for (k = 0; k < 100; k++)
            {
              if (client_list[k].list_id == 0)
                break;

              sprintf(buffer1, "%s %d %s %s %d", "LOGIN", client_list[k].list_id, client_list[k].hostname, client_list[k].ip_addr, client_list[k].port_num);
              send(fdaccept, buffer1, strlen(buffer1), 0) == strlen(buffer1);
            }

            // printf("Login Done\n");

            fflush(stdout);
          }
          else
          {
            //Process incoming data from existing clients here ...
            int receiver = 0;
            char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
            memset(buffer, '\0', BUFFER_SIZE);

            if (recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0)
            {
              close(sock_index);
              printf("Remote Host terminated connection!\n");

              /*Remove from watched list */
              FD_CLR(sock_index, &master_list);
            }
            else
            {
              printf("Message:%s\n", buffer);
              char *fn_cp = malloc(strlen(buffer) + 1);
              strcpy(fn_cp, buffer);
              char *cmd = strtok(fn_cp, " ");
              printf("cmd:%s\n", cmd);

              if (strcmp(cmd, "LOGOUT\n") == 0)
              {
                logout(client_list, sock_index);
                //printf("Logout was received");
              }
              else if (strcmp(cmd, "REFRESH\n") == 0)
              {
                refresh(client_list, sock_index);
              }
              else if (strcmp(cmd, "BLOCK") == 0)
              {
                count_block_indexes = blockClient(buffer, count_block_indexes, blocked_struct_list, sock_index);
              }
              else if (strcmp(cmd, "BLOCKED") == 0)
              {
                char *blocker_ip = strtok(NULL, "");
                getBlockedList(blocker_ip, blocked_struct_list, client_list, count_block_indexes);
              }
              else if (strcmp(cmd, "UNBLOCK") == 0)
              {
                unblockClient(buffer, count_block_indexes, blocked_struct_list, sock_index);
              }
              else if (strcmp(cmd, "SEND") == 0)
              {
                // SEND message
                char *send_msg = malloc(strlen(buffer) + 1);
                strcpy(send_msg, buffer);
                char *send_cmd = strtok(send_msg, " ");
                char *ip = strtok(NULL, " ");
                char *message1 = strtok(NULL, "");
                max_receiver_ips = sendMessage(ip, message1, client_list, blocked_struct_list, message_buffer_list, sock_index, count_block_indexes, max_receiver_ips);
              }
              else if (strcmp(cmd, "BROADCAST") == 0)
              {
                // BROADCAST message
                char *message1 = strtok(NULL, "");
                max_receiver_ips = broadcast(message1, client_list, blocked_struct_list, message_buffer_list, sock_index, count_block_indexes, max_receiver_ips);
              }
              else if (strcmp(cmd, "BUFFER") == 0)
              {
                sendBufferedMessages(client_list, message_buffer_list, sock_index, max_receiver_ips, count_block_indexes, blocked_struct_list);
              }
              else
              {
                printf("\nCMD not found, %s", cmd);
              }
            }

            free(buffer);
          }
        }
      }
    }
  }
}

void refresh(struct client_details client_list[100], int sock_index)
{
  char *buffer1 = (char *)malloc(sizeof(char) * BUFFER_SIZE);
  int i;
  //printf("%s",cmd );
  for (i = 0; i < 100; i++)
  {
    if (client_list[i].list_id == 0)
      break;
    sprintf(buffer1, "%s %d %s %s %d", "REFRESH", client_list[i].list_id, client_list[i].hostname, client_list[i].ip_addr, client_list[i].port_num);
    send(sock_index, buffer1, strlen(buffer1), 0);
  }
}

int sendBufferedMessages(struct client_details client_list[100],  struct message_details message_buffer_list[5], int sock_index, int max_receiver_ips, int count_block_indexes, struct blocked_details blocked_struct_list[5])
{
  char *logged_in_ip;
  for (int k = 0; k < 100; k++)
  {
    if (client_list[k].fdaccept == sock_index)
    {
      logged_in_ip = malloc(strlen(client_list[k].ip_addr) + 1);
      strcpy(logged_in_ip, client_list[k].ip_addr);
      break;
    }
  }

  for (int a = 0; a < 5; a++)
  {

    if (message_buffer_list[a].ip_receiver != NULL && strcmp(message_buffer_list[a].ip_receiver, logged_in_ip) == 0)
    {
      int count = message_buffer_list[a].count;
      for (int j = 0; j < message_buffer_list[a].count; j++)
      {
        if (message_buffer_list[a].messages_list[j] != NULL)
        {
          max_receiver_ips = sendMessage(logged_in_ip, message_buffer_list[a].messages_list[j], client_list, blocked_struct_list, message_buffer_list, sock_index, count_block_indexes, max_receiver_ips);

          message_buffer_list[a].messages_list[j] = '\0';
          message_buffer_list[a].sender_ip[j] = '\0';
          count--;
        }
      }
      message_buffer_list[a].count = count;
    }
  }

  successMessage("LOGIN");
  endMessage("LOGIN");

  return max_receiver_ips;
}

void logout(struct client_details client_list[100], int sock_index)
{
  int i = 0;
  for (i = 0; i < 100; i++)
  {
    if (client_list[i].fdaccept == sock_index)
      break;
  }
  //printf("%s will be removed\n", client_list[i].hostname);
  remove_from_list(client_list, client_list[i].fdaccept);
  adjust_list_ids(client_list);
  display(client_list);
}

void unblockClient(char *buffer, int count_block_indexes, struct blocked_details blocked_struct_list[5], int sock_index)
{
  char *block = malloc(strlen(buffer) + 1);
  strcpy(block, buffer);
  char *block_cmd = strtok(block, " ");
  char *ip = strtok(NULL, "");

  //for UNBlock
  int unblocked = 0;
  ip[strlen(ip) - 1] = '\0';
  for (int k = 0; k < count_block_indexes; k++)
  {
    if (blocked_struct_list[k].fd_accept == sock_index)
    {
      for (int j = 0; j < blocked_struct_list[k].count; j++)
      {
        if (strcmp(blocked_struct_list[k].blocked_ips_list[j], ip) == 0)
        {
          unblocked = 1;
          if (j == blocked_struct_list[k].count - 1)
          {
            blocked_struct_list[k].blocked_ips_list[j] = '\0';
          }
          else
          {
            // blocked_struct_list[k].blocked_ips_list[j] = malloc(strlen(blocked_struct_list[k].blocked_ips_list[j+1]) + 1);
            strcpy(blocked_struct_list[k].blocked_ips_list[j], blocked_struct_list[k].blocked_ips_list[j + 1]);
          }
        }
      }
    }
    if (unblocked == 1)
    {
      blocked_struct_list[k].count--;
      break;
    }
    else
    {
      printf("%d Cannot Unblock %s\n", sock_index, ip);
    }
  }
}

void getBlockedList(char *blocker_ip, struct blocked_details blocked_struct_list[5], struct client_details client_list[100], int count_block_indexes)
{
  int blocker_fdaccept = -1;
  blocker_ip[strlen(blocker_ip) - 1] = '\0';
  for (int ind = 0; ind < 100; ind++)
  {
    if (strcmp(client_list[ind].ip_addr, blocker_ip) == 0)
    {
      blocker_fdaccept = client_list[ind].fdaccept;
      break;
    }
  }
  int counter = 1;
  for (int k = 0; k < count_block_indexes; k++)
  {
    if (blocker_fdaccept == blocked_struct_list[k].fd_accept)
    {
      for (int i = 0; i < 100; i++)
      {
        for (int j = 0; j < blocked_struct_list[k].count; j++)
        {
          if (strcmp(blocked_struct_list[k].blocked_ips_list[j], client_list[i].ip_addr) == 0)
          {
            printf("%-5d%-35s%-20s%-8d\n", counter, client_list[i].hostname, client_list[i].ip_addr, client_list[i].port_num);
            counter++;
          }
        }
      }
    }
  }
}

int blockClient(char *buffer, int count_block_indexes, struct blocked_details blocked_struct_list[5], int sock_index)
{

  char *block = malloc(strlen(buffer) + 1);
  strcpy(block, buffer);
  char *block_cmd = strtok(block, " ");
  char *ip = strtok(NULL, "");

  //for Block
  int end_outer_loop = 0;
  for (int k = 0; k < 5 && end_outer_loop == 0; k++)
  {

    if ((count_block_indexes == k) || (blocked_struct_list[k].fd_accept != sock_index))
    {
      // if ip not current
      if (count_block_indexes == k)
      {
        // reached end, insert new ip
        ip[strlen(ip) - 1] = '\0';
        blocked_struct_list[k].fd_accept = sock_index;
        blocked_struct_list[k].count++;
        blocked_struct_list[k].blocked_ips_list[0] = malloc(strlen(ip) + 1);
        strcpy(blocked_struct_list[k].blocked_ips_list[0], ip);
        end_outer_loop = 1;
        count_block_indexes++;
        break;
      }
      else if (blocked_struct_list[k].fd_accept != sock_index)
      {
        // continue to next
        continue;
      }
    }
    else
    {
      // if ip is already in list, insert new blocked ip in list
      for (int j = 0; j < 4; j++)
      {
        if ((blocked_struct_list[k].count == j) || (strcmp(blocked_struct_list[k].blocked_ips_list[j], ip) == 0))
        {
          if (blocked_struct_list[k].count == j)
          {
            // reached end insert new ip
            ip[strlen(ip) - 1] = '\0';
            blocked_struct_list[k].blocked_ips_list[j] = malloc(strlen(ip) + 1);
            strcpy(blocked_struct_list[k].blocked_ips_list[j], ip);
            blocked_struct_list[k].count++;
          }
          else if (strcmp(blocked_struct_list[k].blocked_ips_list[j], ip) == 0)
          {
            // blocked ip already in list
            printf("%d Already blocked %s ip\n", blocked_struct_list[k].fd_accept, ip);
          }
          end_outer_loop = 1;
          break;
        }
        else
        {
          printf("Not end of list nor already blocked\n");
          continue;
        }
      }
    }
  }

  return count_block_indexes;
}

int sendMessage(char *ip, char *message1, struct client_details client_list[100], struct blocked_details blocked_struct_list[5], struct message_details message_buffer_list[5], int sock_index, int count_block_indexes, int max_receiver_ips)
{

  printf("ip:%s\n", ip);
  printf("message1:%s\n", message1);
  int receiver = 0;
  int do_not_send = 0;
  char *sender_ip_message;
  for (int i = 0; i < 100; i++)
  {

    if (strcmp(client_list[i].ip_addr, ip) == 0)
    {
      for (int k = 0; k < count_block_indexes; k++)
      {
        if (blocked_struct_list[k].fd_accept == sock_index)
        {
          for (int j = 0; j < blocked_struct_list[k].count; j++)
          {
            if (strcmp(blocked_struct_list[k].blocked_ips_list[j], ip) == 0)
            {
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

  char *sender_ip;
  for (int i = 0; i < 100; i++)
  {
    if (client_list[i].fdaccept == sock_index)
    {
      sender_ip_message = malloc(strlen(client_list[i].ip_addr) + 1);
      strcpy(sender_ip_message, client_list[i].ip_addr);
      sender_ip = malloc(strlen(client_list[i].ip_addr) + 1);
      strcpy(sender_ip, client_list[i].ip_addr);
      break;
    }
  }

  strcat(sender_ip_message, " ");
  strcat(sender_ip_message, message1);
  int message_sent = 0;
  if (do_not_send == 0)
  {
    if (receiver > 0)
    {
      if (send(receiver, sender_ip_message, strlen(sender_ip_message), 0) == strlen(sender_ip_message))
      {
        message_sent = 1;
      }

      fflush(stdout);
    }
  }
  else
  {
    printf("%d has blocked ip %s. Hence message not sent\n", sock_index, ip);
  }

  if (message_sent == 0)
  {
    int message_buffer_updated = 0;
    for (int k = 0; k < max_receiver_ips; k++)
    {
      if (strcmp(message_buffer_list[k].ip_receiver, ip) == 0)
      {
        message_buffer_list[k].messages_list[message_buffer_list[k].count] = malloc(strlen(sender_ip_message) + 1);
        strcpy(message_buffer_list[k].messages_list[message_buffer_list[k].count], sender_ip_message);
        message_buffer_list[k].sender_ip[message_buffer_list[k].count] = malloc(strlen(sender_ip) + 1);
        strcpy(message_buffer_list[k].sender_ip[message_buffer_list[k].count], sender_ip);
        message_buffer_list[k].count++;
        message_buffer_updated = 1;
        break;
      }
    }
    if (message_buffer_updated == 0)
    {
      message_buffer_list[max_receiver_ips].ip_receiver = malloc(strlen(ip) + 1);
      strcpy(message_buffer_list[max_receiver_ips].ip_receiver, ip);
      message_buffer_list[max_receiver_ips].messages_list[message_buffer_list[max_receiver_ips].count] = malloc(strlen(sender_ip_message) + 1);
      strcpy(message_buffer_list[max_receiver_ips].messages_list[message_buffer_list[max_receiver_ips].count], sender_ip_message);
      message_buffer_list[max_receiver_ips].count++;
      message_buffer_list[max_receiver_ips].sender_ip[message_buffer_list[max_receiver_ips].count] = malloc(strlen(sender_ip) + 1);
      strcpy(message_buffer_list[max_receiver_ips].sender_ip[message_buffer_list[max_receiver_ips].count], sender_ip);
      max_receiver_ips++;
      message_buffer_updated = 1;
    }
  }
  else
  {
    successMessage("RELAYED");
    cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", sender_ip, ip, message1);
    endMessage("RELAYED");
  }

  return max_receiver_ips;
}

int broadcast(char *message1, struct client_details client_list[100], struct blocked_details blocked_struct_list[5], struct message_details message_buffer_list[5], int sock_index, int count_block_indexes, int max_receiver_ips)
{
  char *sender_ip_message;
  for (int ind = 0; ind < 100; ind++)
  {
    if (client_list[ind].fdaccept == sock_index)
    {
      sender_ip_message = malloc(strlen(client_list[ind].ip_addr) + 1);
      strcpy(sender_ip_message, client_list[ind].ip_addr);
      break;
    }
  }
  for (int i = 0; i < 100; i++)
  {

    if (client_list[i].list_id != 0 && sock_index != client_list[i].fdaccept && client_list[i].fdaccept != 0)
    {
      int do_not_send = 0;
      for (int k = 0; k < count_block_indexes; k++)
      {
        if (blocked_struct_list[k].fd_accept == sock_index)
        {
          for (int j = 0; j < blocked_struct_list[k].count; j++)
          {
            if (strcmp(blocked_struct_list[k].blocked_ips_list[j], client_list[i].ip_addr) == 0)
            {
              do_not_send = 1;
              break;
            }
          }
        }
      }
      int message_sent = 0;
      if (do_not_send == 0)
      {
        if (send(client_list[i].fdaccept, sender_ip_message, strlen(sender_ip_message), 0) == strlen(sender_ip_message))
          printf("Done!\n");
        message_sent = 1;
        fflush(stdout);
      }
      else
      {
        printf("%d has blocked ip %s. Hence message not sent\n", sock_index, client_list[i].ip_addr);
      }
      if (message_sent == 0)
      {
        int message_buffer_updated = 0;
        for (int k = 0; k < max_receiver_ips; k++)
        {
          if (strcmp(message_buffer_list[k].ip_receiver, client_list[i].ip_addr) == 0)
          {
            // printf("1 Updating buffer list for %s with message %s\n", message_buffer_list[k].ip_receiver, sender_ip_message);
            message_buffer_list[k].messages_list[message_buffer_list[k].count] = malloc(strlen(sender_ip_message) + 1);
            strcpy(message_buffer_list[k].messages_list[message_buffer_list[k].count], sender_ip_message);
            // append sender ip in front of message with a delemiter
            message_buffer_list[k].count++;
            message_buffer_updated = 1;
            break;
          }
        }
        if (message_buffer_updated == 0)
        {
          // printf("2 Updating buffer list for %s with message %s\n", message_buffer_list[max_receiver_ips].ip_receiver, sender_ip_message);
          message_buffer_list[max_receiver_ips].ip_receiver = malloc(strlen(client_list[i].ip_addr) + 1);
          strcpy(message_buffer_list[max_receiver_ips].ip_receiver, client_list[i].ip_addr);
          message_buffer_list[max_receiver_ips].messages_list[message_buffer_list[max_receiver_ips].count] = malloc(strlen(sender_ip_message) + 1);
          strcpy(message_buffer_list[max_receiver_ips].messages_list[message_buffer_list[max_receiver_ips].count], sender_ip_message);
          // append sender ip in front of message with a delemiter
          message_buffer_list[max_receiver_ips].count++;
          max_receiver_ips++;
          message_buffer_updated = 1;
        }
      }
     
    }
  }
  return max_receiver_ips;
}

void sort(struct client_details client_list[100])
{
  int i, j;
  struct client_details temp;

  for (i = 0; i <= 100; i++)
  {
    if (client_list[i + 1].port_num == 0)
    {
      break;
    }
    for (j = 0; j <= 100 - i; j++)
    {
      //printf("%d %d %d\n",i,j,client_list[j].port_num);
      if (client_list[j + 1].port_num == 0)
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
void adjust_list_ids(struct client_details client_list[100])
{
  int i;

  for (i = 0; i < 100; i++)
  {
    if (client_list[i].list_id == 0)
      break;
    else
      client_list[i].list_id = i + 1;
  }
}

void remove_from_list(struct client_details client_list[100], int key)
{
  int i, pointer;
  struct client_details temp;
  for (i = 0; i < 100; i++)
  {
    if (client_list[i].fdaccept == key)
      break;
  }
  client_list[i].list_id = 0;
  client_list[i].port_num = 0;

  for (pointer = i + 1; pointer < 100; pointer++)
  {
    if (client_list[pointer].list_id == 0)
      break;
    else
    {
      temp = client_list[pointer - 1];
      client_list[pointer - 1] = client_list[pointer];
      client_list[pointer] = temp;
    }
  }
}

void display(struct client_details client_list[100])
{
  int i;
  for (i = 0; i < 100; i++)
  {
    if (client_list[i].list_id == 0)
      break;
    successMessage("LIST");
    cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", client_list[i].list_id, client_list[i].hostname, client_list[i].ip_addr, client_list[i].port_num);
    endMessage("LIST");
  }
}
