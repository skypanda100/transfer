//
// Created by root on 19-6-30.
//

#ifndef CTRANSFER_DEF_H
#define CTRANSFER_DEF_H
#include <stdio.h>
#include <sys/inotify.h>
#include <limits.h>

#define BUF_LEN         4096
#define BUFFER_SIZE     4096
#define CIPHER1         "@mtt@start"
#define CIPHER2         "@mtt@transferring"
#define CIPHER3         "@mtt@end"

typedef struct st_file_info
{
    long remain_size;
    FILE *fp;
}file_info;

typedef struct st_conf
{
    char host[32];
    int port;
    char key[128];
    char log[1024];
    char ignore[128];
    char src_dir[1024];
    char dst_dir[1024];
    int buffer_size;
}conf;

typedef struct st_dir_watch
{
    int wd;
    char wpath[PATH_MAX];
}dir_watch;

typedef struct st_notification
{
    int notify_fd;
    dir_watch *dir_watch_ptr;
    int dir_watch_ptr_len;
}notification;

#endif //CTRANSFER_DEF_H
