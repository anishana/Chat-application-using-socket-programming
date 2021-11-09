struct client_details {
  int list_id;
  char hostname[50];
  char ip_addr[100];
  int fdaccept;
  int port_num;
  int num_msg_rcv;
  int num_msg_sent;
  int is_logged_in;
};

struct blocked_details {
  int count;
  int fd_accept;
  char *ip_addr;
  char * blocked_ips_list[4]; // = {NULL, NULL, NULL, NULL};
};

struct message_details {
  int count;
  char * ip_receiver;
  char * messages_list[100];
  char * sender_ip[100];
};



int run_server(int argc, char **argv);
void receive_msg(int argc, char **argv);
int sendMessage(char * ip, char * message1, struct client_details client_list[100], struct client_details stats[5], struct blocked_details blocked_struct_list[5], struct message_details message_buffer_list[5], int sock_index, int count_block_indexes, int max_receiver_ips, int isBuffer, int bufferIndex);
int broadcast(char * message1, struct client_details client_list[100], struct client_details stats[5], struct blocked_details blocked_struct_list[5], struct message_details message_buffer_list[5], int sock_index, int count_block_indexes, int max_receiver_ips);
void unblockClient(char *buffer, int count_block_indexes, struct client_details client_list[100], struct blocked_details blocked_struct_list[5], int sock_index);
int blockClient(char *buffer, struct client_details client_list[100], int count_block_indexes, struct blocked_details blocked_struct_list[5], int sock_index);
void getBlockedList(char *blocker_ip, struct blocked_details blocked_struct_list[5], struct client_details client_list[100], int count_block_indexes);

void display(struct client_details client_list[100]);
void sort(struct client_details client_list[100]);
void adjust_list_ids(struct client_details client_list[100]);
void remove_from_list(struct client_details client_list[100], int key);
void logout(struct client_details client_list[100], struct client_details stats[5], int sock_index);
void refresh(struct client_details client_list[100], int sock_index);
int sendBufferedMessages(struct client_details client_list[100], struct client_details stats[5], struct message_details message_buffer_list[5],  int sock_index, int max_receiver_ips, int count_block_indexes, struct blocked_details blocked_struct_list[5], int index);
void getStatistics(struct client_details stats[5]);
void initialiseLists(struct client_details client_list[100], struct client_details stats[5], struct blocked_details blocked_struct_list[5], struct message_details message_buffer_list[5]);
void sendAcknowledgement(char *cmd_str, int fdaccept);
void sendList(struct client_details client_list[100], int sock_index);