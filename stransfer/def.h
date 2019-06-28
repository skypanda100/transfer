//
// Created by root on 19-6-27.
//

#ifndef STRANSFER_DEF_H
#define STRANSFER_DEF_H
#include <sys/time.h>
#include <limits.h>
#include <sys/inotify.h>
#include "log.h"

#define CONTENT_SIZE    1024
#define BUFFER_SIZE     sizeof(int) + CONTENT_SIZE
#define BACKLOG         64
#define CLIENT_MAX      1024
#define CIPHER          "@mtt@ is my cat"

typedef struct st_package
{
    int size;
    char content[CONTENT_SIZE];
}package;

typedef struct st_conf
{
    char host[32];
    int port;
    char key[128];
    char log[1024];
}conf;

#endif //STRANSFER_DEF_H
