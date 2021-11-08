#include <stdbool.h>

void getIp(void);
void getAuthor(void);
void getPort(char *port);
void successMessage(char *cmdstr);
void endMessage(char *cmdstr);
void errorMessage(char *cmdstr);
bool validateIp(char *ip);