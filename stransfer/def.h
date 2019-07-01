//
// Created by zhengdongtian on 19-6-27.
//

#ifndef STRANSFER_DEF_H
#define STRANSFER_DEF_H
#include <stdio.h>

#define BUFFER_SIZE     4096
#define BACKLOG         64
#define CLIENT_MAX      1024
#define CIPHER          "@mtt@ is my cat"

typedef struct st_file_info
{
    int is_login;
    long remain_size;
    FILE *fp;
}file_info;

typedef struct st_conf
{
    char host[32];
    int port;
    char key[128];
    char log[1024];
}conf;

#endif //STRANSFER_DEF_H
