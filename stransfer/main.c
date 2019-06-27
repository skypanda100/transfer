#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#define BACKLOG 5     //完成三次握手但没有accept的队列的长度
#define CONCURRENT_MAX 8   //应用层同时可以处理的连接
#define SERVER_PORT 80
#define BUFFER_SIZE 1024
#define QUIT_CMD ".quit\n"
#define TRANSFER_MSG   "transfer_end"

int main(int argc, const char * argv[])
{
    char input_msg[BUFFER_SIZE];
    char recv_msg[BUFFER_SIZE];
    //本地地址
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&(server_addr.sin_zero), 8);
    //创建socket
    int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock_fd == -1)
    {
        perror("socket error");
        return 1;
    }
    //绑定socket
    int bind_result = bind(server_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(bind_result == -1)
    {
        perror("bind error");
        return 1;
    }
    //listen
    if(listen(server_sock_fd, BACKLOG) == -1)
    {
        perror("listen error");
        return 1;
    }

    //create epollfd
    int events_len = CONCURRENT_MAX + 2;
    int epollfd = epoll_create(events_len);
    if(epollfd == -1)
    {
        fprintf(stderr, "create epollfd failed!\n");
        exit(-1);
    }

    //clientfd
    int clientfds[CONCURRENT_MAX];
    for(int i = 0;i < CONCURRENT_MAX;i++)
    {
        clientfds[i] = -1;
    }

    // client fp
    FILE *clientfps[CONCURRENT_MAX];
    for(int i = 0;i < CONCURRENT_MAX;i++)
    {
        clientfps[i] = NULL;
    }

    //add event to kernel
    struct epoll_event stdin_event;
    stdin_event.events = EPOLLIN;
    stdin_event.data.fd = STDIN_FILENO;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &stdin_event);

    struct epoll_event server_event;
    server_event.events = EPOLLIN;
    server_event.data.fd = server_sock_fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, server_sock_fd, &server_event);

    //get events from kernel
    int timeout = 1000;
    struct epoll_event events[events_len];

    //do epoll
    while(1)
    {
        int ret = epoll_wait(epollfd, events, events_len, timeout);
        if(ret < 0)
        {
            perror("epoll 出错\n");
            continue;
        }
        else if(ret == 0)
        {
            printf("epoll 超时\n");
            continue;
        }
        else
        {
            for(int i = 0;i < ret;i++)
            {
                int fd = events->data.fd;
                if(fd == server_sock_fd && (events->events & server_event.events))
                {
                    //有新的连接请求
                    struct sockaddr_in client_address;
                    socklen_t address_len;
                    int client_sock_fd = accept(server_sock_fd, (struct sockaddr *)&client_address, &address_len);
                    printf("new connection client_sock_fd = %d\n", client_sock_fd);
                    if(client_sock_fd > 0)
                    {
                        int index = -1;
                        for(int client_i = 0;client_i < CONCURRENT_MAX;client_i++)
                        {
                            if(clientfds[client_i] == -1)
                            {
                                index = client_i;
                                clientfds[client_i] = client_sock_fd;

                                // add event to kernel
                                struct epoll_event client_event;
                                client_event.events = EPOLLIN;
                                client_event.data.fd = client_sock_fd;
                                epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sock_fd, &client_event);

                                break;
                            }
                        }
                        if(index >= 0)
                        {
                            printf("新客户端(%d)加入成功 %s:%d\n", index, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                        }
                        else
                        {
                            bzero(input_msg, BUFFER_SIZE);
                            strcpy(input_msg, "服务器加入的客户端数达到最大值,无法加入!\n");
                            send(client_sock_fd, input_msg, BUFFER_SIZE, 0);
                            printf("客户端连接数达到最大值，新客户端加入失败 %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                        }
                    }
                }
                else if(fd == STDIN_FILENO && (events->events & stdin_event.events))
                {
                    bzero(input_msg, BUFFER_SIZE);
                    fgets(input_msg, BUFFER_SIZE, stdin);
                    //输入“.quit"则退出服务器
                    if(strcmp(input_msg, QUIT_CMD) == 0)
                    {
                        exit(0);
                    }
                    for(int client_i = 0;client_i < CONCURRENT_MAX;client_i++)
                    {
                        if(clientfds[client_i] > 0)
                        {
                            printf("向客户端(%d)发送消息\n", client_i);
                            send(clientfds[client_i], input_msg, BUFFER_SIZE, 0);
                        }
                    }
                }
                else
                {
                    //处理某个客户端过来的消息
                    bzero(recv_msg, BUFFER_SIZE);
                    long byte_num = recv(events[i].data.fd, recv_msg, BUFFER_SIZE, 0);
                    if(byte_num > 0)
                    {
                        for(int client_i = 0;client_i < CONCURRENT_MAX;client_i++)
                        {
                            if(clientfds[client_i] == events[i].data.fd)
                            {
                                if(clientfps[client_i] == NULL)
                                {
                                    FILE *fp = fopen(recv_msg, "wb");
                                    if(fp == NULL)
                                    {
                                        printf("客户端(%d), create file failed:%s\n", client_i, recv_msg);
                                    }
                                    else
                                    {
                                        clientfps[client_i] = fp;
                                        send(clientfds[client_i], TRANSFER_MSG, strlen(TRANSFER_MSG), 0);
                                    }
                                }
                                else
                                {
                                    if(strcmp(recv_msg, TRANSFER_MSG) == 0)
                                    {
                                        fclose(clientfps[client_i]);
                                        clientfps[client_i] = NULL;
                                        printf("transfer end\n");
                                    }
                                    else
                                    {
                                        fwrite(recv_msg, 1, byte_num, clientfps[client_i]);
                                        send(clientfds[client_i], TRANSFER_MSG, strlen(TRANSFER_MSG), 0);
                                    }
                                }
                                break;
                            }
                        }
                    }
                    else if(byte_num < 0)
                    {
                        for(int client_i = 0;client_i < CONCURRENT_MAX;client_i++)
                        {
                            if(clientfds[client_i] == events[i].data.fd)
                            {
                                printf("从客户端(%d)接受消息出错.\n", client_i);
                                break;
                            }
                        }
                    }
                    else
                    {
                        // delete event in kernel
                        struct epoll_event client_event;
                        client_event.events = EPOLLIN;
                        client_event.data.fd = events[i].data.fd;
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, &client_event);

                        for(int client_i = 0;client_i < CONCURRENT_MAX;client_i++)
                        {
                            if(clientfds[client_i] == events[i].data.fd)
                            {
                                clientfds[client_i] = -1;
                                printf("客户端(%d)退出了.\n", client_i);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    close(epollfd);

    return 0;
}