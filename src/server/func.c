#include "func.h"

char info[] = "Welcome to the study server 'myserver'";

int info_cmd(int sfd, char *cmd) {
    char buffer[64];
    int success = 0;
    errno = 0;

    cmd += 4;
    if (*cmd != '\0') {
        if (*cmd != ' ') {
            snprintf(buffer, sizeof(buffer), "\x01server: %s: command not found...", cmd - 4);
            send(sfd, buffer, strlen(buffer), 0);
        }
        else {
            while (*cmd == ' ') cmd++;
            if (*cmd != '\0') {
                strcpy(buffer, "\x01server: INFO: too many arguments");
                send(sfd, buffer, strlen(buffer), 0);
            }
            else success = 1;
        }
    }
    else success = 1;

    if (success) send(sfd, info, strlen(info), 0);
    if (errno == EPIPE) return 1;
    return 0;
}

int quit_cmd(int sfd, char *cmd) {
    char buffer[64];
    int success = 0;
    errno = 0;

    cmd += 4;
    if (*cmd != '\0') {
        if (*cmd != ' ') {
            snprintf(buffer, sizeof(buffer), "\x01server: %s: command not found...", cmd - 4);
            send(sfd, buffer, strlen(buffer), 0);
        }
        else {
            while (*cmd == ' ') cmd++;
            if (*cmd != '\0') {
                strcpy(buffer, "\x01server: QUIT: too many arguments");
                send(sfd, buffer, strlen(buffer), 0);
            }
            else success = 1;
        }
    }
    else success = 1;

    if (success) {
        strcpy(buffer, "\x06Goodbye");
        send(sfd, buffer, strlen(buffer), 0);
    }
    if (errno == EPIPE) return 1;
    if (success) return 2;
    return 0;
}

int echo_cmd(int sfd, char *cmd) {
    char buffer[BUFFER_SIZE];
    int success = 0;
    errno = 0;

    cmd += 4;
    if (*cmd != '\0') {
        if (*cmd != ' ') {
            snprintf(buffer, sizeof(buffer), "\x01server: %s: command not found...", cmd - 4);
            send(sfd, buffer, strlen(buffer), 0);
        }
        else {
            while (*cmd == ' ') cmd++;
            success = 1;
        }
    }
    else success = 1;

    if (success) {
        strlen(cmd) == 0 ? strcpy(buffer, "\x07") : strcpy(buffer, cmd);
        send(sfd, buffer, strlen(buffer), 0);
    }
    if (errno == EPIPE) return 1;
    return 0;
}

int cd_cmd(int sfd, char *cmd, char *work_dir) {
    char buffer[BUFFER_SIZE];
    char new_path[PATH_MAX];
    char resolved_path[PATH_MAX];
    char *arg;
    errno = 0;

    cmd += 2;
    if (*cmd != '\0') {
        if (*cmd != ' ') {
            snprintf(buffer, sizeof(buffer), "\x01server: %s: command not found...", cmd - 2);
            send(sfd, buffer, strlen(buffer), 0);
        }
        else {
            while (*cmd == ' ') cmd++;
            if (*cmd != '\0') {
                arg = cmd;
                while (*cmd != ' ' && *cmd != '\0') cmd++;
                if (*cmd == ' ') {
                    *cmd = '\0';
                    cmd++;
                    while (*cmd == ' ') cmd++;
                }

                if (*cmd == '\0') {
                    if (strcmp(arg, "/") == 0) {
                        strcpy(work_dir, root_dir);
                        strcpy(buffer, "\x05");
                        strcpy(buffer + 1, work_dir + root_dir_len);
                        send(sfd, buffer, strlen(buffer), 0);
                        if (errno == EPIPE) return 1;
                        else return 0;
                    }
                    
                    snprintf(new_path, sizeof(new_path), "%s/%s", work_dir, arg);

                    if (realpath(new_path, resolved_path) == NULL) {
                        switch (errno) {
                            case ENOENT:
                                snprintf(buffer, sizeof(buffer), "\x01server: CD: %s: no such directory", arg);
                                break;
                            case ENOTDIR:
                                snprintf(buffer, sizeof(buffer), "\x01server: CD: %s: not a directory", arg);
                                break;
                            case EACCES:
                                snprintf(buffer, sizeof(buffer), "\x01server: CD: %s: permission denied", arg);
                                break;
                            default:
                                snprintf(buffer, sizeof(buffer), "\x01server: CD: %s: undefined error", arg);
                                break;
                        }

                        send(sfd, buffer, strlen(buffer), 0);
                        if (errno == EPIPE) return 1;
                        else return 0;
                    }

                    struct stat st;
                    if (stat(resolved_path, &st) == -1 || !S_ISDIR(st.st_mode)) {
                        snprintf(buffer, sizeof(buffer), "\x01server: CD: %s: not a directory", arg);
                        send(sfd, buffer, strlen(buffer), 0);
                        if (errno == EPIPE) return 1;
                        return 0;
                    }

                    if (strncmp(resolved_path, root_dir, root_dir_len) != 0 ||
                        (resolved_path[root_dir_len] != '/' && resolved_path[root_dir_len] != '\0')) {
                        strcpy(work_dir, root_dir);
                        strcpy(buffer, "\x05");
                        strcpy(buffer + 1, work_dir + root_dir_len);
                        send(sfd, buffer, strlen(buffer), 0);
                        if (errno == EPIPE) return 1;
                        else return 0;
                    }

                    strcpy(work_dir, resolved_path);
                    strcpy(buffer, "\x05");
                    strcpy(buffer + 1, work_dir + root_dir_len);
                    send(sfd, buffer, strlen(buffer), 0);
                }
                else {
                    strcpy(buffer, "\x01server: CD: too many arguments");
                    send(sfd, buffer, strlen(buffer), 0);
                }
            }
            else {
                strcpy(buffer, "\x01server: CD: missing argument");
                send(sfd, buffer, strlen(buffer), 0);
            }
        }
    }
    else {
        strcpy(buffer, "\x01server: CD: missing argument");
        send(sfd, buffer, strlen(buffer), 0);
    }

    if (errno == EPIPE) return 1;
    return 0;
}

int list_cmd(int sfd, char *cmd, char *work_dir) {
    char buffer[(PATH_MAX << 1) + 32];
    int success = 0;
    errno = 0;

    cmd += 4;
    if (*cmd != '\0') {
        if (*cmd != ' ') {
            snprintf(buffer, sizeof(buffer), "\x01server: %s: command not found...", cmd - 4);
            send(sfd, buffer, strlen(buffer), 0);
        }
        else {
            while (*cmd == ' ') cmd++;
            if (*cmd != '\0') {
                strcpy(buffer, "\x01server: LIST: too many arguments");
                send(sfd, buffer, strlen(buffer), 0);
            }
            else success = 1;
        }
    }
    else success = 1;

    if (errno == EPIPE) return 1;

    if (success) {
        DIR *dir = opendir(work_dir);
        struct dirent *entry;
        struct stat st;
        char path[PATH_MAX];
        char link_target[PATH_MAX >> 1];
        ssize_t len;

        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            snprintf(path, sizeof(path), "%s/%s", work_dir, entry->d_name);

            if (lstat(path, &st) == -1) continue;

            if (S_ISLNK(st.st_mode)) {
                len = readlink(path, link_target, sizeof(link_target) - 1);
                if (len != -1) {
                    link_target[len] = '\0';

                    struct stat target_st;
                    if (stat(path, &target_st) == 0) {
                        if (S_ISLNK(target_st.st_mode)) {
                            snprintf(buffer, sizeof(buffer), "\x03%s -->> %s", entry->d_name, link_target);
                        }
                        else {
                            snprintf(buffer, sizeof(buffer), "\x03%s --> %s", entry->d_name, link_target);
                        }
                    }
                    else {
                        snprintf(buffer, sizeof(buffer), "\x03%s --> %s", entry->d_name, link_target);
                    }
                }
            }
            else if (S_ISDIR(st.st_mode)) snprintf(buffer, sizeof(buffer), "\x03%s/", entry->d_name);
            else snprintf(buffer, sizeof(buffer), "\x03%s", entry->d_name);

            send(sfd, buffer, strlen(buffer), 0);
            if (errno == EPIPE) return 1;
        }

        closedir(dir);
        strcpy(buffer, "\x04");
        send(sfd, buffer, strlen(buffer), 0);
        if (errno == EPIPE) return 1;
    }

    return 0;
}

int unknown_cmd(int sfd, char *cmd) {
    char buffer[64];
    errno = 0;
    
    if (*cmd == '\0') strcpy(buffer, "\x02");
    else {
        char *arg = cmd;
        while (*arg != ' ' && *arg != '\0') arg++;
        *arg = '\0';
        snprintf(buffer, sizeof(buffer), "\x01server: %s: command not found...", cmd);
    }
    
    send(sfd, buffer, strlen(buffer), 0);
    if (errno == EPIPE) return 1;
    return 0;
}