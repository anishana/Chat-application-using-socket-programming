#include <stdbool.h>



int run_client(int arg, char **argv);
int connect_to_host(char *server_ip, char *server_port, char *client_port);
int receive_msg_from_server(int server);
bool validatePort(char *port);
void initialiseListsClient();
bool validateIpInList(char *ip);