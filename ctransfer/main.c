#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#define CONTENT_SIZE    1024
#define BUFFER_SIZE     sizeof(int) + CONTENT_SIZE
#define CIPHER          "@mtt@ is my cat"

typedef struct st_package
{
    int size;
    char content[CONTENT_SIZE];
}package;

int main(int argc, const char * argv[])
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(11332);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&(server_addr.sin_zero), 8);

    int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock_fd == -1)
    {
        perror("socket error");
        return 1;
    }
    char recv_msg[BUFFER_SIZE];
    char input_msg[BUFFER_SIZE];
    char buffer[BUFFER_SIZE] = {0};
    char content[CONTENT_SIZE] = {0};
    FILE *fp = NULL;
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
                bzero(content, CONTENT_SIZE);
                int len = strlen(input_msg);
                memcpy(buffer, &len, sizeof(int));
                memcpy(buffer + sizeof(int), input_msg, len);
                if(send(server_sock_fd, buffer, BUFFER_SIZE, 0) == -1)
                {
                    perror("发送path消息出错!\n");
                }
                else
                {
                    fp = fopen("/home/zhengdongtian/CLionProjects/ctransfer/main.c", "rb");
                }
            }
            if(FD_ISSET(server_sock_fd, &client_fd_set))
            {
                bzero(recv_msg, BUFFER_SIZE);
                long byte_num = recv(server_sock_fd, recv_msg, BUFFER_SIZE, 0);
                if(byte_num > 0)
                {
                    if(strncmp(recv_msg, CIPHER, strlen(CIPHER)) == 0)
                    {
                        if(fp != NULL)
                        {
                            while(!feof(fp))
                            {
                                bzero(buffer, BUFFER_SIZE);
                                bzero(content, CONTENT_SIZE);
                                int len = fread(content, 1, CONTENT_SIZE, fp);
                                memcpy(buffer, &len, sizeof(int));
                                memcpy(buffer + sizeof(int), content, CONTENT_SIZE);
                                if(send(server_sock_fd, buffer, len, 0) == -1)
                                {
                                    printf("send failed\n");
                                    break;
                                }
                            }
                            fclose(fp);
                            fp = NULL;

                            bzero(buffer, BUFFER_SIZE);
                            bzero(content, CONTENT_SIZE);
                            int len = strlen(CIPHER);
                            memcpy(buffer, &len, sizeof(int));
                            memcpy(buffer + sizeof(int), CIPHER, len);
                            if(send(server_sock_fd, buffer, BUFFER_SIZE, 0) == -1)
                            {
                                perror("发送transfer end消息出错!\n");
                            }
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