#ifndef SERVEH_H
#define SERVER_H

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <arpa/inet.h>

#define MAX_COUNT_CLIENTS 4

typedef struct {
    int active;
    int sfd;
    char work_dir[PATH_MAX];
} client_s;

void get_time_prefix(char *buffer, size_t size);
void start_server(int argc, char **argv);
void main_worker(int sfd);

#endif //SERVER_H