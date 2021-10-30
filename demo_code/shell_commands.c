#include <stdio.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/logger.h"
#include "shell_commands.h"
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void getIp()
{
	char hostbuffer[256];
    	char *IPbuffer;
    	struct hostent *host_entry;
    	int hostname;
  
    	// To retrieve hostname
    	hostname = gethostname(hostbuffer, sizeof(hostbuffer));
  
    	// To retrieve host information
    	host_entry = gethostbyname(hostbuffer);
  
    	// To convert an Internet network
    	// address into ASCII string
    	IPbuffer = inet_ntoa(*((struct in_addr*)
                           host_entry->h_addr_list[0]));
	cse4589_print_and_log("[IP:SUCCESS]\n");  	
    	cse4589_print_and_log("IP:%s\n", IPbuffer);
	cse4589_print_and_log("[IP:END]\n");    	
}


void getAuthor() 
{
	cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
	cse4589_print_and_log("I, sachinge, have read and understood the course academic integrity policy.\n");
	cse4589_print_and_log("[AUTHOR:END]\n");	

}

void getPort(char *port)
{
	cse4589_print_and_log("[PORT:SUCCESS]\n");
	cse4589_print_and_log("PORT:%s\n", port);
	cse4589_print_and_log("[PORT:END]\n");	
}

