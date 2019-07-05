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
//    int keepIdle = 60;
//    int keepInterval = 10;
//    int keepCount = 10;
//    setsockopt(client_sock_fd, SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle));
//    setsockopt(client_sock_fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
//    setsockopt(client_sock_fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));

    if(client_sock_fd == -1)
    {
        char *err_ptr = strerror(errno);
        LOG("create socket failed: %s", err_ptr);
        fprintf(stderr, "create socket failed: %s\n", err_ptr);
        return -1;
    }

    int ret = connect(client_sock_fd, (struct sockaddr *)&client_address, sizeof(struct sockaddr_in));
    if(ret == -1)
    {
        char *err_ptr = strerror(errno);
        LOG("connect failed: %s", err_ptr);
        fprintf(stderr, "connect failed: %s\n", err_ptr);
        return -1;
    }

    if(send(client_sock_fd, cf.key, strlen(cf.key), 0) == -1)
    {
        char *err_ptr = strerror(errno);
        LOG("login failed: %s", err_ptr);
        fprintf(stderr, "login failed: %s\n", err_ptr);
        return -1;
    }
    else
    {
        char server_msg[BUFFER_SIZE] = {0};
        long server_msg_size = receive_from_server(client_sock_fd, server_msg, BUFFER_SIZE);
        if(server_msg_size > 0)
        {
            if(strncmp(server_msg, CIPHER2, strlen(CIPHER2)) == 0)
            {
                LOG("login successfully!");
            }
            else
            {
                LOG("login failed!");
                fprintf(stderr, "login failed\n");
                return -1;
            }
        }
        else
        {
            char *err_ptr = strerror(errno);
            LOG("login failed: %s", err_ptr);
            fprintf(stderr, "login failed: %s\n", err_ptr);
            return -1;
        }
    }

    return client_sock_fd;
}

long receive_from_server(int client_sock_fd, char *buffer_ptr, int buffer_size)
{
    memset(buffer_ptr, 0, buffer_size);
    long server_msg_size = recv(client_sock_fd, buffer_ptr, buffer_size, 0);

    return server_msg_size;
}
