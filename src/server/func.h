#ifndef FUNC_H
#define FUNC_H

#define _XOPEN_SOURCE 700

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

extern char info[];
extern char root_dir[PATH_MAX];
extern size_t root_dir_len;

int info_cmd(int sfd, char *cmd);
int quit_cmd(int sfd, char *cmd);
int echo_cmd(int sfd, char *cmd);
int cd_cmd(int sfd, char *cmd, char *work_dir);
int list_cmd(int sfd, char *cmd, char *work_dir);
int unknown_cmd(int sfd, char *cmd);

#endif FNCH_H