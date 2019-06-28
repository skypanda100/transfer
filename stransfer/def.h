//
// Created by root on 19-6-27.
//

#ifndef STRANSFER_DEF_H
#define STRANSFER_DEF_H
#include <sys/time.h>
#include <limits.h>
#include <sys/inotify.h>
#include "log.h"

#define BUFFER_SIZE     1024
#define BACKLOG         64
#define CLIENT_MAX      1024

typedef struct st_conf
{
    char host[32];
    int port;
    char key[128];
    char log[1024];
}conf;

#endif //STRANSFER_DEF_H
