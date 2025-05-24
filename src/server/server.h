#ifndef SERVEH_H
#define SERVER_H

#include "func.h"
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdint.h>

#define MAX_COUNT_CLIENTS 4
#define NORMAL_CLOSE 0
#define UNEXPECTED_CLOSE 1

typedef struct {
    int active;
    int sfd;
    char work_dir[PATH_MAX + 1];
} client_s;

void get_time_prefix(char *buffer, size_t size);
int start_server(int argc, char **argv);
void main_worker(int sfd);
void* client_worker(void *arg);
void client_close(client_s *client, struct sockaddr_in *sin, int how);
void int_handler(int signo);

#endif //SERVER_H