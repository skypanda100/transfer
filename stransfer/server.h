//
// Created by root on 19-6-27.
//

#ifndef STRANSFER_SERVER_H
#define STRANSFER_SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "def.h"
#include "log.h"

int create_socket();
int get_client_socket(int server_sock_fd);
long recv_from_client(int client_sock_fd, char *buffer_ptr, size_t buffer_size);

#endif //STRANSFER_SERVER_H
