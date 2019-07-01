#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <string.h>
#include "config.h"
#include "server.h"

conf cf;
static int server_sock_fd = -1;
static int event_poll_fd = -1;
static int event_poll_size = CLIENT_MAX + 1;
static int client_sock_fd_a[CLIENT_MAX];
static file_info client_file_info_a[CLIENT_MAX];

void init_server_socket();
void init_event_poll_fd();
void init_client();
void register_event_poll(int fd, int is_client_fd);
void cancel_event_poll(int fd);
void clear_client_file_info(int index);
void do_event_poll();
int index_of_client_sock_fd_a(int fd);
int create_dir(const char *path_ptr);

int main(int argc, const char * argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "please input conf path!\n");
        exit(-1);
    }

    config(argv[1]);
    init_server_socket();
    init_event_poll_fd();
    init_client();

    register_event_poll(server_sock_fd, 0);
    do_event_poll();

    return 0;
}

void init_server_socket()
{
    server_sock_fd = create_socket();
    if(server_sock_fd == -1)
    {
        exit(-1);
    }
}

void init_event_poll_fd()
{
    event_poll_fd = epoll_create(event_poll_size);
    if(event_poll_fd == -1)
    {
        LOG("%s", "create event poll fd failed!");
        exit(-1);
    }
}

void init_client()
{
    for(int i = 0;i < CLIENT_MAX;i++)
    {
        client_sock_fd_a[i] = -1;
    }

    for(int i = 0;i < CLIENT_MAX;i++)
    {
        client_file_info_a[i].remain_size = 0;
        client_file_info_a[i].fp = NULL;
    }
}

void register_event_poll(int fd, int is_client_fd)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    epoll_ctl(event_poll_fd, EPOLL_CTL_ADD, fd, &event);

    if(is_client_fd)
    {
        int index = -1;
        for(int client_i = 0;client_i < CLIENT_MAX;client_i++)
        {
            if(client_sock_fd_a[client_i] == -1)
            {
                index = client_i;
                client_sock_fd_a[client_i] = fd;
                clear_client_file_info(client_i);
                break;
            }
        }
        if(index == -1)
        {
            // do something when clients is full
        }
    }
}

void cancel_event_poll(int fd)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    epoll_ctl(event_poll_fd, EPOLL_CTL_DEL, fd, &event);

    for(int client_i = 0;client_i < CLIENT_MAX;client_i++)
    {
        if(client_sock_fd_a[client_i] == fd)
        {
            client_sock_fd_a[client_i] = -1;
            clear_client_file_info(client_i);
            LOG("client fd is %d, quit!", fd);
            break;
        }
    }
}

void clear_client_file_info(int index)
{
    client_file_info_a[index].remain_size = 0;
    if(client_file_info_a[index].fp != NULL)
    {
        fclose(client_file_info_a[index].fp);
        client_file_info_a[index].fp = NULL;
    }
}

int index_of_client_sock_fd_a(int fd)
{
    int index = -1;
    for(int client_i = 0;client_i < CLIENT_MAX;client_i++)
    {
        if(client_sock_fd_a[client_i] == fd)
        {
            index = client_i;
            break;
        }
    }
    return index;
}

int create_dir(const char *path_ptr)
{
    char path[PATH_MAX] = {0};
    strcpy(path, path_ptr);
    int len = strlen(path);
    for(int i = len - 1;i >= 0;i--)
    {
        if(path[i] != '/')
        {
            path[i] = '\0';
        }
        else
        {
            break;
        }
    }
    return make_dir(path);
}

void do_event_poll()
{
    char buffer[BUFFER_SIZE] = {0};
    struct epoll_event event_poll_event[event_poll_size];
    int timeout = 3 * 1000;
    while(1)
    {
        int ret = epoll_wait(event_poll_fd, event_poll_event, event_poll_size, timeout);
        if(ret < 0)
        {
            LOG("%s", "event poll wait failed!");
            continue;
        }
        else if(ret == 0)
        {
            LOG("%s", "event poll wait timeout!");
            continue;
        }
        else
        {
            for(int i = 0;i < ret;i++)
            {
                if(event_poll_event[i].data.fd == server_sock_fd && (event_poll_event[i].events & EPOLLIN))
                {
                    // 有新的连接请求
                    int client_sock_fd = get_client_socket(server_sock_fd);
                    if(client_sock_fd > 0)
                    {
                        register_event_poll(client_sock_fd, 1);
                    }
                }
                else
                {
                    // 处理某个客户端过来的消息
                    int receive_size = receive_from_client(event_poll_event[i].data.fd, buffer, BUFFER_SIZE);
                    if(receive_size > 0)    // normal
                    {
                        int client_i = index_of_client_sock_fd_a(event_poll_event[i].data.fd);
                        if(client_i >= 0)
                        {
                            if(client_file_info_a[client_i].fp == NULL)
                            {
                                char *path_ptr = buffer + sizeof(long);
                                if(create_dir(path_ptr) == 0)
                                {
                                    FILE *fp = fopen(path_ptr, "wb");
                                    if(fp == NULL)
                                    {
                                        LOG("client fd is %d, create file failed: %s", client_sock_fd_a[client_i], path_ptr);
                                    }
                                    else
                                    {
                                        memcpy(&(client_file_info_a[client_i].remain_size), buffer, sizeof(long));
                                        client_file_info_a[client_i].fp = fp;
                                        send(client_sock_fd_a[client_i], CIPHER, strlen(CIPHER), 0);
                                        LOG("client fd is %d, create file successfully: %s, %ld", client_sock_fd_a[client_i], path_ptr, client_file_info_a[client_i].remain_size);
                                    }
                                }
                                else
                                {
                                    LOG("client fd is %d, create dir failed: %s", client_sock_fd_a[client_i], path_ptr);
                                }
                            }
                            else
                            {
                                fwrite(buffer, 1, receive_size, client_file_info_a[client_i].fp);
                                client_file_info_a[client_i].remain_size -= receive_size;
                                if(client_file_info_a[client_i].remain_size == 0)
                                {
                                    // end
                                    LOG("client fd is %d, write file successfully!", client_sock_fd_a[client_i]);
                                    clear_client_file_info(client_i);
                                    send(client_sock_fd_a[client_i], CIPHER, strlen(CIPHER), 0);
                                }
                                else if(client_file_info_a[client_i].remain_size < 0)
                                {
                                    // error
                                    LOG("client fd is %d, write file failed!", client_sock_fd_a[client_i]);
                                    clear_client_file_info(client_i);
                                }
                            }
                        }
                    }
                    else if(receive_size < 0)   // error
                    {
                        int client_i = index_of_client_sock_fd_a(event_poll_event[i].data.fd);
                        if(client_i >= 0)
                        {
                            LOG("client fd is %d, receive message failed!", client_sock_fd_a[client_i]);
                            clear_client_file_info(client_i);
                        }
                    }
                    else    // close
                    {
                        cancel_event_poll(event_poll_event[i].data.fd);
                    }
                }
            }
        }
    }
}
