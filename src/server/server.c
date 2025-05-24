#include "server.h"

int sfd;
client_s clients[MAX_COUNT_CLIENTS];
char root_dir[PATH_MAX];
size_t root_dir_len;
sigset_t set;

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

    root_dir_len = strlen(root_dir);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int err = getaddrinfo("localhost", argv[1], &hints, &res);
    if (err) {
        fprintf(stderr, "server: getaddrinfo: %s\n", gai_strerror(err));
        exit(1);
    }

    sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sfd == -1) {
        perror("server: socket");
        freeaddrinfo(res);
        exit(1);
    }

    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(sfd, res->ai_addr, res->ai_addrlen) != 0) {
        perror("server: bind");
        freeaddrinfo(res);
        close(sfd);
        exit(1);
    }

    if (listen(sfd, SOMAXCONN) != 0) {
        perror("server: listen");
        freeaddrinfo(res);
        close(sfd);
        exit(1);
    }

    struct sockaddr_in *sin = (struct sockaddr_in *)res->ai_addr;

    char buf[32];
    get_time_prefix(buf, sizeof(buf));
    printf("%s server listeting on %s:%d\n", buf, inet_ntoa(sin->sin_addr), ntohs(sin->sin_port));

    freeaddrinfo(res);

    signal(SIGPIPE, SIG_IGN);

    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = int_handler;
    sigaction(SIGINT, &sa, NULL);

    main_worker();
}

void main_worker() {
    pthread_t thread;
    pthread_attr_t tattr;
    char buffer[BUFFER_SIZE];

    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

    while (1) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd == -1) continue;

        int m = -1;
        for (int i = 0; i < MAX_COUNT_CLIENTS; i++) {
            if (clients[i].active == 0) {
                m = i;
                break;
            }
        }

        if (m == -1) {
            strcpy(buffer, "SERVER IS BUSY");
            send(cfd, buffer, strlen(buffer), 0);
            close(cfd);
            continue;
        }

        clients[m].active = 1;
        clients[m].sfd = cfd;
        strcpy(clients[m].work_dir, root_dir);

        int ret = pthread_create(&thread, &tattr, client_worker, &clients[m]);
        if (ret != 0) {
            perror("server: pthread_create");
            clients[m].active = 0;
            memset(clients[m].work_dir, 0, sizeof(clients[m].work_dir));
            close(cfd);
        }
    }
}

void *client_worker(void *arg) {
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    client_s *client = (client_s *)arg;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    off_t offset;
    int res;

    getpeername(client->sfd, (struct sockaddr *)&sin, &len);
    get_time_prefix(buffer, sizeof(buffer));
    printf("%s client %s:%d connected to the server\n", buffer, inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

    send(client->sfd, info, strlen(info), 0);
    if (errno == EPIPE) client_close(client, &sin, UNEXPECTED_CLOSE);

    while (1) {
        ssize_t bytes = recv(client->sfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes == 0) client_close(client, &sin, UNEXPECTED_CLOSE);
        buffer[bytes] = '\0';

        offset = 0;
        while (buffer[offset] == ' ') offset++;

        if (strncmp(buffer + offset, "INFO", 4) == 0) res = info_cmd(client->sfd, buffer + offset);
        else if (strncmp(buffer + offset, "QUIT", 4) == 0) res = quit_cmd(client->sfd, buffer + offset);
        else if (strncmp(buffer + offset, "ECHO", 4) == 0) res = echo_cmd(client->sfd, buffer + offset);
        else if (strncmp(buffer + offset, "CD", 2) == 0) res = cd_cmd(client->sfd, buffer + offset, client->work_dir);
        else if (strncmp(buffer + offset, "LIST", 4) == 0) res = list_cmd(client->sfd, buffer + offset, client->work_dir);
        else res = unknown_cmd(client->sfd, buffer + offset);

        if (res == 1) client_close(client, &sin, UNEXPECTED_CLOSE);
        if (res == 2) client_close(client, &sin, NORMAL_CLOSE);
    }
}

void client_close(client_s *client, struct sockaddr_in *sin, int how) {
    char buffer[32];

    client->active = 0;
    memset(client->work_dir, 0, sizeof(client->work_dir));
    close(client->sfd);

    get_time_prefix(buffer, sizeof(buffer));
    printf("%s client %s:%d %sdisconnected from the server\n",
            buffer, 
            inet_ntoa(sin->sin_addr), 
            ntohs(sin->sin_port), 
            how == NORMAL_CLOSE ? "" : "unexpected ");

    pthread_exit(NULL);
}

void int_handler(int signo) {
    for (int i = 0; i < MAX_COUNT_CLIENTS; i++) {
        if (clients[i].active) close(clients[i].sfd);
    }
    close(sfd);

    char buffer[32];
    get_time_prefix(buffer, sizeof(buffer));
    printf("\n%s server has been shut down\n", buffer);

    exit(0);
}