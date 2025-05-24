#include "client.h"

char work_dir[PATH_MAX];
int sfd;

void connect_client(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (err) {
        fprintf(stderr, "client: getaddrinfo: %s\n", gai_strerror(err));
        exit(1);
    }

    sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sfd == -1) {
        perror("client: socket");
        freeaddrinfo(res);
        exit(1);
    }

    if (connect(sfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("client: connect");
        freeaddrinfo(res);
        close(sfd);
        exit(1);
    }

    strcpy(work_dir, "");

    signal(SIGPIPE, SIG_IGN);

    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = int_handle;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    freeaddrinfo(res);

    server_interact();
}

void server_interact() {
    char buffer[BUFFER_SIZE];

    ssize_t bytes = recv(sfd, buffer, sizeof(buffer) - 1, 0);
    if (bytes == 0) close_server(sfd);

    buffer[bytes] = '\0';
    if (strcmp(buffer, "SERVER IS BUSY") == 0) {
        printf("%s\n", buffer);
        close_server(sfd);
    }
    else printf("%s\n", buffer);

    while (1) {
        printf("%s> ", strcmp(work_dir, "") == 0 ? "/" : work_dir);
        fflush(stdout);
        read_cmd(buffer, sizeof(buffer));
        if (strlen(buffer) == 0) continue;

        errno = 0;
        send(sfd, buffer, strlen(buffer), 0);
        if (errno == EPIPE) {
            printf("SERVER IS NOT RESPONDING\n");
            close_server();
        }

        bytes = recv(sfd, buffer, sizeof(buffer), 0);
        if (bytes == 0) {
            printf("SERVER IS NOT RESPONDING\n");
            close_server();
        }

        buffer[bytes] = '\0';

        switch (buffer[0]) {
            case '\x01':
                printf("%s\n", buffer + 1);
                break;
            case '\x02':
                break;
            case '\x03':
                while (1) {
                    if (buffer[0] == '\x04') break;

                    printf("%s\n", buffer + 1);

                    bytes = recv(sfd, buffer, sizeof(buffer), 0);
                    if (bytes == 0) {
                        printf("SERVER IS NOT RESPONDING\n");
                        close_server();
                    }

                    buffer[bytes] = '\0';
                }
                break;
            case '\x05':
                strcpy(work_dir, buffer + 1);
                break;
            case '\x06':
                printf("%s\n", buffer + 1);
                close_server();
                break;
            default:
                printf("%s\n", buffer);
                break;
        }
    }
}

void read_cmd(char *buffer, size_t max_len) {
    int c;
    size_t i = 0;

    while((c = getchar()) != EOF) {
        if (c == '\n') break;
        if (c == '\t') c = ' ';

        if (i < max_len - 1) {
            buffer[i++] = c;
        }
        else {
            while ((c = getchar()) != EOF && c != '\n');
            break;
        }
    }

    buffer[i] = '\0';
}

void close_server() {
    close(sfd);
    exit(0);
}

void int_handle(int signo) {
    printf("\n");
    close_server();
}