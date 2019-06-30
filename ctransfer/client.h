//
// Created by root on 19-6-30.
//

#ifndef CTRANSFER_CLIENT_H
#define CTRANSFER_CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "def.h"
#include "log.h"

int create_socket();
long receive_from_server(int client_sock_fd, char *buffer_ptr, int buffer_size);

#endif //CTRANSFER_CLIENT_H
