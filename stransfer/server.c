//
// Created by zhengdongtian on 19-6-27.
//
#include "server.h"

extern conf cf;

int create_socket()
{
    // 本地地址
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(cf.port);
    server_address.sin_addr.s_addr = inet_addr(cf.host);
    bzero(&(server_address.sin_zero), 8);
    // 创建socket
    int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock_fd == -1)
    {
        char *err_ptr = strerror(errno);
        LOG("create socket failed: %s", err_ptr);
        fprintf(stderr, "create socket failed: %s\n", err_ptr);
        return -1;
    }
    // 绑定socket
    int bind_result = bind(server_sock_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    if(bind_result == -1)
    {
        char *err_ptr = strerror(errno);
        LOG("socket bind failed: %s", err_ptr);
        fprintf(stderr, "socket bind failed: %s\n", err_ptr);
        return -1;
    }
    // listen
    if(listen(server_sock_fd, BACKLOG) == -1)
    {
        char *err_ptr = strerror(errno);
        LOG("listen socket failed: %s", err_ptr);
        fprintf(stderr, "listen socket failed: %s\n", err_ptr);
        return -1;
    }

    return server_sock_fd;
}

int get_client_socket(int server_sock_fd)
{
    struct sockaddr_in client_address;
    socklen_t address_len;
    int client_sock_fd = accept(server_sock_fd, (struct sockaddr *)&client_address, &address_len);
//    int enable = 1;
//    setsockopt(client_sock_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));

    LOG("new client want to join us, ip is %s, port is %d, fd is %d", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), client_sock_fd);

    return client_sock_fd;
}

long receive_from_client(int client_sock_fd, char *buffer_ptr, int buffer_size)
{
    memset(buffer_ptr, 0, buffer_size);
    long client_msg_size = recv(client_sock_fd, buffer_ptr, buffer_size, 0);

    return client_msg_size;
}
