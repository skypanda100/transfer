//
// Created by root on 19-6-30.
//
#include "client.h"

extern conf cf;

int create_socket()
{
    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(cf.port);
    client_address.sin_addr.s_addr = inet_addr(cf.host);
    bzero(&(client_address.sin_zero), 8);

    int client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
//    int enable = 1;
//    setsockopt(client_sock_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));

    if(client_sock_fd == -1)
    {
        LOG("create socket failed: %s", strerror(errno));
        return -1;
    }

    int ret = connect(client_sock_fd, (struct sockaddr *)&client_address, sizeof(struct sockaddr_in));
    if(ret == -1)
    {
        LOG("connect failed: %s", strerror(errno));
        return -1;
    }

    return client_sock_fd;
}

long receive_from_server(int client_sock_fd, char *buffer_ptr, int buffer_size)
{
    memset(buffer_ptr, 0, buffer_size);
    long server_msg_size = recv(client_sock_fd, buffer_ptr, buffer_size, 0);

    return server_msg_size;
}
