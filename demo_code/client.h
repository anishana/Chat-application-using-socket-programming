#include <stdbool.h>

struct client_details
{
    int list_id;
    char hostname[50];
    char ip_addr[100];
    int fdaccept;
    int port_num;
};

int run_client(int arg, char **argv);
int connect_to_host(char *server_ip, char *server_port, char *client_port);
int receive_msg_from_server(int server);
bool validateIp(char *ip);
bool validatePort(char *port);
void initialiseListsClient();
void display_list(struct client_details client_list[100]);
void clear(struct client_details client_list[100]);