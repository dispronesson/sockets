#include "server.h"

char root_dir[PATH_MAX];

void get_time_prefix(char *buffer, size_t size){
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm *tm_info = localtime(&tv.tv_sec);

    int millis = tv.tv_usec / 1000;

    snprintf(buffer, size, "%04d.%02d.%02d-%02d:%02d:%02d.%03d",
             tm_info->tm_year + 1900,
             tm_info->tm_mon + 1,
             tm_info->tm_mday,
             tm_info->tm_hour,
             tm_info->tm_min,
             tm_info->tm_sec,
             millis);
}

void start_server(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <rootdir>\n", argv[0]);
        exit(1);
    }

    char *str = realpath(argv[2], root_dir);
    if (!str) {
        perror("server: realpath");
        exit(1);
    }

    struct stat st;
    if (stat(root_dir, &st) != 0) {
        perror("server: stat");
        exit(1);
    }

    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "server: %s is not directory\n", root_dir);
        exit(1);
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE;

    int err = getaddrinfo("localhost", argv[1], &hints, &res);
    if (err) {
        fprintf(stderr, "server: getaddrinfo: %s\n", gai_strerror(err));
        exit(1);
    }

    int sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sfd == -1) {
        perror("server: socket");
        exit(1);
    }

    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(sfd, res->ai_addr, res->ai_addrlen) != 0) {
        perror("server: bind");
        exit(1);
    }

    if (listen(sfd, SOMAXCONN) != 0) {
        perror("server: listen");
        exit(1);
    }

    struct sockaddr_in *sin = (struct sockaddr_in *)res->ai_addr;

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(res->ai_family, &sin->sin_addr, ip_str, sizeof(ip_str));

    char buf[32];
    get_time_prefix(buf, sizeof(buf));

    printf("%s server listeting on %s:%d\n", buf, ip_str, htons(sin->sin_port));

    freeaddrinfo(res);
}

void main_worker(int sfd) {
    client_s clients[MAX_COUNT_CLIENTS];
    pthread_t thread;
    pthread_attr_t pattr;

    pthread_attr_init(&pattr);
    pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_DETACHED);
}