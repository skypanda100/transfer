#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "def.h"
#include "log.h"
#include "client.h"
#include "notify.h"

conf cf;
char **transfer_file_ptr_ptr = NULL;
int transfer_file_len = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t thread = 0;
static int client_sock_fd = -1;

void init_client_socket();
void reconnect();
void transfer();

int main(int argc, const char * argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "please input conf path!\n");
        exit(-1);
    }

    config(argv[1]);
    init_client_socket();
    // create a thread that loop files and transfer it
    pthread_create(&thread, NULL, (void *)&transfer, NULL);
    // watch src dir
    watch();

    return 0;
}

void init_client_socket()
{
    client_sock_fd = create_socket();
    if(client_sock_fd == -1)
    {
        exit(-1);
    }
}

void reconnect()
{
    while(1)
    {
        client_sock_fd = create_socket();
        if(client_sock_fd != -1)
        {
            break;
        }
        sleep(5);
    }
}

void transfer()
{
    // select
    fd_set client_fd_set;
    struct timeval tv;
    // socket message
    char buffer[BUFFER_SIZE] = {0};
    char server_msg[BUFFER_SIZE] = {0};
    // transfer file
    FILE *transfer_fp = NULL;
    long transfer_size = 0;
    int transfer_index = 0;
    // temp files
    char **file_ptr_ptr = NULL;
    int file_len = 0;

    for(;;)
    {
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        FD_ZERO(&client_fd_set);
        FD_SET(client_sock_fd, &client_fd_set);

        if(transfer_index >= file_len)
        {
            // free
            for(int i = 0;i < file_len;i++) {
                free(file_ptr_ptr[i]);
                file_ptr_ptr[i] = NULL;
            }
            free(file_ptr_ptr);
            file_ptr_ptr = NULL;
            file_len = 0;

            if(transfer_file_ptr_ptr != NULL)
            {
                if(pthread_mutex_lock(&mutex) != 0)
                {
                    LOG("pthread_mutex_lock failed!");
                }
                transfer_index = 0;
                file_len = transfer_file_len;
                file_ptr_ptr = (char **)malloc(sizeof(char *) * file_len);
                memcpy(file_ptr_ptr, transfer_file_ptr_ptr, sizeof(char *) * transfer_file_len);

                // free
                free(transfer_file_ptr_ptr);
                transfer_file_ptr_ptr = NULL;
                transfer_file_len = 0;
                if(pthread_mutex_unlock(&mutex) != 0)
                {
                    LOG("pthread_mutex_unlock failed!");
                }
            }
        }

        if(file_len > 0)
        {
            if(transfer_fp == NULL)
            {
                char *file_ptr = file_ptr_ptr[transfer_index];
                if(strlen(file_ptr) > 0) {
                    transfer_fp = fopen(file_ptr, "rb");
                    if(transfer_fp != NULL)
                    {
                        fseek(transfer_fp, 0L, SEEK_END);
                        transfer_size = ftell(transfer_fp);
                        fseek(transfer_fp, 0L, SEEK_SET);

                        if(transfer_size > 0)
                        {
                            char dst_file[1024] = {0};
                            strcpy(dst_file, cf.dst_dir);
                            strcat(dst_file, "/");
                            strcat(dst_file, file_ptr + strlen(cf.src_dir));

                            memcpy(buffer, &transfer_size, sizeof(long));
                            memcpy(buffer + sizeof(long), dst_file, strlen(dst_file));
                            if(send(client_sock_fd, buffer, sizeof(long) + strlen(dst_file), 0) == -1)
                            {
                                LOG("send path failed: %s", file_ptr);
                                if (transfer_fp != NULL) {
                                    fclose(transfer_fp);
                                    transfer_fp = NULL;
                                }
                            }
                            else
                            {
                                LOG("send path successfully: %s", file_ptr);
                            }
                        }
                        else
                        {
                            transfer_index++;
                            if (transfer_fp != NULL) {
                                fclose(transfer_fp);
                                transfer_fp = NULL;
                            }
                        }
                    }
                    else
                    {
                        transfer_index++;
                    }
                }
            }
            select(client_sock_fd + 1, &client_fd_set, NULL, NULL, &tv);
            if(FD_ISSET(client_sock_fd, &client_fd_set))
            {
                bzero(server_msg, BUFFER_SIZE);
                long server_msg_size = receive_from_server(client_sock_fd, server_msg, BUFFER_SIZE);
                if(server_msg_size > 0)
                {
                    if(strncmp(server_msg, CIPHER, strlen(CIPHER)) == 0)
                    {
                        if(transfer_fp != NULL && transfer_size > 0)
                        {
                            while(!feof(transfer_fp))
                            {
                                bzero(buffer, BUFFER_SIZE);
                                int send_len = 0;
                                int len = fread(buffer, 1, BUFFER_SIZE, transfer_fp);
                                if(len > transfer_size)
                                {
                                    send_len = transfer_size;
                                }
                                else
                                {
                                    send_len = len;
                                }
                                if(send(client_sock_fd, buffer, send_len, 0) == -1)
                                {
                                    LOG("send file failed!");
                                    break;
                                }
                                transfer_size -= send_len;
                                if(transfer_size == 0)
                                {
                                    break;
                                }
                            }
                        }
                        else
                        {
                            transfer_index++;
                            if (transfer_fp != NULL) {
                                fclose(transfer_fp);
                                transfer_fp = NULL;
                            }
                            LOG("receive from server: receive file successfully!");
                        }
                    }
                }
                else if(server_msg_size < 0)
                {
                    LOG("receive from server failed!");
                }
                else
                {
                    LOG("receive from server: server quit!");
                    reconnect();
                }
            }
        }
    }
}
