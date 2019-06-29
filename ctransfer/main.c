#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#define BUFFER_SIZE     4096
#define CIPHER          "@mtt@ is my cat"

int main(int argc, const char * argv[])
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(11332);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&(server_addr.sin_zero), 8);

    int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    int enable = 1;
//    setsockopt(server_sock_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));

    if(server_sock_fd == -1)
    {
        perror("socket error");
        return 1;
    }
    char recv_msg[BUFFER_SIZE];
    char input_msg[BUFFER_SIZE];
    char buffer[BUFFER_SIZE] = {0};
    FILE *fp = NULL;
    long file_size = 0;
    if(connect(server_sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == 0)
    {
        fd_set client_fd_set;
        struct timeval tv;

        while(1)
        {
            tv.tv_sec = 20;
            tv.tv_usec = 0;
            FD_ZERO(&client_fd_set);
            FD_SET(STDIN_FILENO, &client_fd_set);
            FD_SET(server_sock_fd, &client_fd_set);

            select(server_sock_fd + 1, &client_fd_set, NULL, NULL, &tv);
            if(FD_ISSET(STDIN_FILENO, &client_fd_set))
            {
                bzero(input_msg, BUFFER_SIZE);
                fgets(input_msg, BUFFER_SIZE, stdin);
                for(int i = strlen(input_msg) - 1;i >= 0;i--)
                {
                    if(input_msg[i] == '\n' || input_msg[i] == '\r')
                    {
                        input_msg[i] = 0;
                    }
                    else
                    {
                        break;
                    }
                }
                bzero(buffer, BUFFER_SIZE);

                fp = fopen("/home/zhengdongtian/app/transfer/ctransfer/main.c", "rb");
                fseek(fp, 0L, SEEK_END);
                file_size = ftell(fp);
                fseek(fp, 0L, SEEK_SET);

                int len = strlen(input_msg);
                memcpy(buffer, &file_size, sizeof(long));
                memcpy(buffer + sizeof(long), input_msg, len);
                if(send(server_sock_fd, buffer, sizeof(long) + len, 0) == -1)
                {
                    perror("发送path消息出错!\n");
                    if(fp != NULL)
                    {
                        fclose(fp);
                        fp = NULL;
                    }
                }
            }
            if(FD_ISSET(server_sock_fd, &client_fd_set))
            {
                bzero(recv_msg, BUFFER_SIZE);
                long byte_num = recv(server_sock_fd, recv_msg, BUFFER_SIZE, 0);
                printf("byte_num = %ld\n", byte_num);
                if(byte_num > 0)
                {
                    if(strncmp(recv_msg, CIPHER, strlen(CIPHER)) == 0)
                    {
                        if(fp != NULL)
                        {
                            while(!feof(fp))
                            {
                                bzero(buffer, BUFFER_SIZE);
                                int send_len = 0;
                                int len = fread(buffer, 1, BUFFER_SIZE, fp);
                                if(len > file_size)
                                {
                                    send_len = file_size;
                                }
                                else
                                {
                                    send_len = len;
                                }
                                if(send(server_sock_fd, buffer, send_len, 0) == -1)
                                {
                                    printf("send failed\n");
                                    break;
                                }
                                file_size -= send_len;
                                if(file_size == 0)
                                {
                                    break;
                                }
                            }
                            fclose(fp);
                            fp = NULL;
                        }
                        else
                        {
                            printf("receive from server: receive file successfully!\n");
                        }
                    }
                }
                else if(byte_num < 0)
                {
                    printf("接受消息出错!\n");
                }
                else
                {
                    printf("服务器端退出!\n");
                    exit(0);
                }
            }
        }
        //}
    }
    return 0;
}