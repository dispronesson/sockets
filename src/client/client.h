#ifndef CLIENT_H
#define CLIENT_H

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 4096

void connect_client(int argc, char **argv);
void server_interact();
void read_cmd(char *buffer, size_t max_len);
void close_server();
void int_handle(int signo);

#endif //CLIENT_H