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
static int client_fd[CLIENT_MAX];
static FILE *client_fp[CLIENT_MAX];

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
        client_fd[i] = -1;
    }

    for(int i = 0;i < CLIENT_MAX;i++)
    {
        client_fp[i] = NULL;
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
            if(client_fd[client_i] == -1)
            {
                index = client_i;
                client_fd[client_i] = fd;
                if(client_fp[client_i] != NULL)
                {
                    fclose(client_fp[client_i]);
                    client_fp[client_i] = NULL;
                }
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
        if(client_fd[client_i] == fd)
        {
            client_fd[client_i] = -1;
            if(client_fp[client_i] != NULL)
            {
                fclose(client_fp[client_i]);
                client_fp[client_i] = NULL;
            }
            LOG("client fd is %d, quit!", fd);
            break;
        }
    }
}

void do_event_poll()
{
    package pkg;
    struct epoll_event event_poll_event[event_poll_size];
    int timeout = 20 * 1000;
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
                    int receive_ret = receive_from_client(event_poll_event[i].data.fd, &pkg);
                    if(receive_ret == 0)
                    {
                        for(int client_i = 0;client_i < CLIENT_MAX;client_i++)
                        {
                            if(client_fd[client_i] == event_poll_event[i].data.fd)
                            {
                                if(client_fp[client_i] == NULL)
                                {
                                    FILE *fp = fopen(pkg.content, "wb");
                                    if(fp == NULL)
                                    {
                                        LOG("client fd is %d, create file failed: %s", client_fd[client_i], pkg.content);
                                    }
                                    else
                                    {
                                        client_fp[client_i] = fp;
                                        send(client_fd[client_i], CIPHER, strlen(CIPHER), 0);
                                    }
                                }
                                else
                                {
                                    if(strncmp(pkg.content, CIPHER, strlen(CIPHER)) == 0)
                                    {
                                        fclose(client_fp[client_i]);
                                        client_fp[client_i] = NULL;
                                        LOG("client fd is %d, write file successfully!", client_fd[client_i]);
                                    }
                                    else
                                    {
                                        fwrite(pkg.content, 1, pkg.size, client_fp[client_i]);
                                    }
                                }
                                break;
                            }
                        }
                    }
                    else if(receive_ret == -2)
                    {
                        for(int client_i = 0;client_i < CLIENT_MAX;client_i++)
                        {
                            if(client_fd[client_i] == event_poll_event[i].data.fd)
                            {
                                LOG("client fd is %d, receive message failed!", client_fd[client_i]);
                                break;
                            }
                        }
                    }
                    else
                    {
                        cancel_event_poll(event_poll_event[i].data.fd);
                    }
                }
            }
        }
    }
}

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