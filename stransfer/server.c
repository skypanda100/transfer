//
// Created by root on 19-6-27.
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
        LOG("%s", "create socket failed!");
        return -1;
    }
    // 绑定socket
    int bind_result = bind(server_sock_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    if(bind_result == -1)
    {
        LOG("%s", "socket bind failed!");
        return -1;
    }
    // listen
    if(listen(server_sock_fd, BACKLOG) == -1)
    {
        LOG("%s", "listen socket failed!");
        return -1;
    }

    return server_sock_fd;
}

int get_client_socket(int server_sock_fd)
{
    struct sockaddr_in client_address;
    socklen_t address_len;
    int client_sock_fd = accept(server_sock_fd, (struct sockaddr *)&client_address, &address_len);
    LOG("new client join in, ip is %s, port is %d, fd is %d)", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), client_sock_fd);

    return client_sock_fd;
}

int receive_from_client(int client_sock_fd, package *pkg_ptr)
{
    memset(pkg_ptr, 0, sizeof(package));
    char client_msg[BUFFER_SIZE] = {0};
    long client_msg_size = recv(client_sock_fd, client_msg, BUFFER_SIZE, 0);
    printf("%d\n", client_msg_size);
    if(client_msg_size == 0)
    {
        // close
        return -1;
    }
    else if(client_msg_size != BUFFER_SIZE)
    {
        // error
        return -2;
    }
    else
    {
        // normal
        memcpy(&(pkg_ptr->size), client_msg, sizeof(int));
        memcpy(&(pkg_ptr->content), client_msg + sizeof(int), CONTENT_SIZE);
    }

    return 0;
}
