//
// Created by root on 19-6-30.
//
#include "notify.h"

extern conf cf;
extern char **transfer_file_ptr_ptr;
extern int transfer_file_len;
extern pthread_mutex_t mutex;
static size_t flags = IN_CREATE | IN_CLOSE_WRITE | IN_MOVED_TO;

static int is_ignore_file(const char *ignore_ptr, const char *file_name_ptr)
{
    if(strlen(ignore_ptr) == 0)
    {
        return 0;
    }

    regex_t rgx;
    if(regcomp(&rgx, ignore_ptr, REG_NOSUB) != 0)
    {
        LOG("invalid regex pattern!");
        regfree(&rgx);
        return 0;
    }

    if(regexec(&rgx, file_name_ptr, 0, NULL, 0) == 0)
    {
        regfree(&rgx);
        return 1;
    }

    regfree(&rgx);

    return 0;
}

static void absolute_path(const notification *ntf_ptr, int wd, const char *name_ptr, char *new_path_ptr)
{
    for(int i = 0;i < ntf_ptr->dir_watch_ptr_len;i++)
    {
        if(wd == ntf_ptr->dir_watch_ptr[i].wd)
        {
            sprintf(new_path_ptr, "%s/%s", ntf_ptr->dir_watch_ptr[i].wpath, name_ptr);
            break;
        }
    }
}

static int add_dir_to_watch_list(notification *ntf_ptr, const char *path_ptr)
{
    int wd = inotify_add_watch(ntf_ptr->notify_fd, path_ptr, flags);
    if(wd == -1)
    {
        LOG("inotify_add_watch failed: %s", path_ptr);
    }
    else
    {
        ntf_ptr->dir_watch_ptr_len += 1;
        dir_watch *s_w = (dir_watch *)realloc(ntf_ptr->dir_watch_ptr, sizeof(dir_watch) * ntf_ptr->dir_watch_ptr_len);
        s_w[ntf_ptr->dir_watch_ptr_len - 1].wd = wd;
        strcpy(s_w[ntf_ptr->dir_watch_ptr_len - 1].wpath, path_ptr);
        ntf_ptr->dir_watch_ptr = s_w;
    }

    return wd;
}

void watch()
{
    notification ntf;
    memset(&ntf, 0, sizeof(notification));

    int nd = inotify_init();
    if(nd == -1)
    {
        LOG("inotify_init failed!");
        ntf.notify_fd = -1;
        ntf.dir_watch_ptr = NULL;
        ntf.dir_watch_ptr_len = 0;
        return;
    }
    ntf.notify_fd = nd;

    int wd = add_dir_to_watch_list(&ntf, cf.src_dir);
    if(wd != -1)
    {
        size_t dirs_len = 0;
        char **dir_ptr_ptr = NULL;
        list_dir(cf.src_dir, &dir_ptr_ptr, &dirs_len);
        for(int i = 0; i < dirs_len;i++)
        {
            add_dir_to_watch_list(&ntf, dir_ptr_ptr[i]);
            LOG("sub dir is %s", dir_ptr_ptr[i]);
        }
        // free
        if(dirs_len > 0)
        {
            for(int i = 0; i < dirs_len;i++)
            {
                free(dir_ptr_ptr[i]);
            }
            free(dir_ptr_ptr);
        }
    }

    // dead loop, watch dirs and files
    for(;;)
    {
        char *temp_buf_ptr = NULL;
        char buf[BUF_LEN] = {0};
        size_t read_len = 0;
        struct inotify_event *event_ptr;

        read_len = read(ntf.notify_fd, buf, BUF_LEN);
        if(read_len == -1)
        {
            LOG("read failed!");
        }

        for(temp_buf_ptr = buf;temp_buf_ptr < buf + read_len;)
        {
            event_ptr = (struct inotify_event *)temp_buf_ptr;
            if(event_ptr->mask & IN_ISDIR)
            {
                if(event_ptr->len > 0)
                {
                    //ignore hidden file
                    if(event_ptr->name[0] != '.')
                    {
                        char new_path[PATH_MAX] = {0};
                        absolute_path(&ntf, event_ptr->wd, event_ptr->name, new_path);
                        if(strlen(new_path) > 0)
                        {
                            add_dir_to_watch_list(&ntf, new_path);
                        }
                    }
                }
            }
            else
            {
                if(event_ptr->mask & (IN_CLOSE_WRITE | IN_MOVED_TO))
                {
                    if(event_ptr->len > 0)
                    {
                        //ignore hidden file
                        if(event_ptr->name[0] != '.')
                        {
                            char src_file_path[PATH_MAX] = {0};
                            absolute_path(&ntf, event_ptr->wd, event_ptr->name, src_file_path);
                            if(is_ignore_file(cf.ignore, src_file_path))
                            {
                                LOG("%s is ignore file!", src_file_path);
                            }
                            else
                            {
                                if(strlen(src_file_path) > 0)
                                {
                                    if(pthread_mutex_lock(&mutex) != 0)
                                    {
                                        LOG("pthread_mutex_lock failed!");
                                    }
                                    transfer_file_len++;
                                    transfer_file_ptr_ptr = (char **)realloc(transfer_file_ptr_ptr, sizeof(char *) * transfer_file_len);
                                    transfer_file_ptr_ptr[transfer_file_len - 1] = strdup(src_file_path);
                                    if(pthread_mutex_unlock(&mutex) != 0)
                                    {
                                        LOG("pthread_mutex_unlock failed!");
                                    }
                                }
                            }
                        }
                    }
                }
            }
            temp_buf_ptr += sizeof(struct inotify_event) + event_ptr->len;
        }
    }
}

