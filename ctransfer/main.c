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
    daemon(0, 1);

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
    int is_wait = 0;
    // socket message
    char buffer[BUFFER_SIZE] = {0};
    char server_msg[BUFFER_SIZE] = {0};
    long server_msg_size = 0;
    // transfer file
    FILE *transfer_fp = NULL;
    long transfer_size = 0;
    long remain_size = 0;
    int transfer_index = 0;
    int is_transferring = 0;
    // temp files
    char **file_ptr_ptr = NULL;
    int file_len = 0;
    // heart beat
    long last_timestamp = timestamp();

    for(;;)
    {
        tv.tv_sec = 0;
        tv.tv_usec = 10;
        FD_ZERO(&client_fd_set);
        FD_SET(client_sock_fd, &client_fd_set);
        bzero(server_msg, BUFFER_SIZE);
        if(is_wait)
        {
            select(client_sock_fd + 1, &client_fd_set, NULL, NULL, NULL);
        }
        else
        {
            select(client_sock_fd + 1, &client_fd_set, NULL, NULL, &tv);
        }
        is_wait = 0;
        if(FD_ISSET(client_sock_fd, &client_fd_set))
        {
            server_msg_size = receive_from_server(client_sock_fd, server_msg, BUFFER_SIZE);
            if(server_msg_size <= 0)
            {
                LOG("receive from server: server quit! server_msg_size is %d, error no is %d!", server_msg_size, errno);
                close(client_sock_fd);
                if(transfer_fp != NULL)
                {
                    fclose(transfer_fp);
                    transfer_fp = NULL;
                }
                reconnect();
                continue;
            }
        }

        if(transfer_index >= file_len)
        {
            // free
            for(int i = 0;i < file_len;i++)
            {
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
                if(strlen(file_ptr) > 0)
                {
                    sleep(cf.delay);
                    //
                    transfer_fp = fopen(file_ptr, "rb");
                    if(transfer_fp != NULL)
                    {
                        fseek(transfer_fp, 0L, SEEK_END);
                        transfer_size = ftell(transfer_fp);
                        remain_size = transfer_size;
                        fseek(transfer_fp, 0L, SEEK_SET);

                        if(transfer_size > 0)
                        {
                            char dst_file[1024] = {0};
                            strcpy(dst_file, cf.dst_dir);
                            strcat(dst_file, "/");
                            strcat(dst_file, file_ptr + strlen(cf.src_dir));

                            int buffer_len = strlen(CIPHER1) + sizeof(long) + strlen(dst_file) + 1;
                            memcpy(buffer, CIPHER1, strlen(CIPHER1));
                            memcpy(buffer + strlen(CIPHER1), &transfer_size, sizeof(long));
                            memcpy(buffer + strlen(CIPHER1) + sizeof(long), dst_file, strlen(dst_file));
                            buffer[buffer_len - 1] = '\0';
                            if(send(client_sock_fd, buffer, buffer_len, 0) == -1)
                            {
                                LOG("send path failed: %s", file_ptr);
                                if(transfer_fp != NULL)
                                {
                                    fclose(transfer_fp);
                                    transfer_fp = NULL;
                                }
                            }
                            else
                            {
                                is_transferring = 1;
                                LOG("send path successfully: %s", file_ptr);
                            }
                            is_wait = 1;
                        }
                        else
                        {
                            transfer_index++;
                            if(transfer_fp != NULL)
                            {
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
            else
            {
                int is_done = 0;
                if(transfer_fp != NULL)
                {
                    if(!feof(transfer_fp))
                    {
                        bzero(buffer, BUFFER_SIZE);
                        int len = fread(buffer, 1, cf.buffer_size, transfer_fp);
                        if(len > 0)
                        {
                            if(send(client_sock_fd, buffer, len, 0) == -1)
                            {
                                LOG("send file failed: %s", strerror(errno));
                            }
                            else
                            {
                                remain_size -= len;
                                if(remain_size <= 0)
                                {
                                    is_done = 1;
                                }
                            }
                        }
                        else
                        {
                            is_done = 1;
                        }
                    }
                    else
                    {
                        is_done = 1;
                    }
                }
                if(is_done)
                {
                    if(!is_transferring)
                    {
                        if(strncmp(server_msg, CIPHER3, strlen(CIPHER3)) == 0)
                        {
                            transfer_index++;
                            if(transfer_fp != NULL)
                            {
                                fclose(transfer_fp);
                                transfer_fp = NULL;
                            }
                        }
                    }
                    else
                    {
                        is_wait = 1;
                        is_transferring = 0;
                        LOG("send file successfully!");
                    }
                }
            }
        }
        else
        {
            // heart beat
            long cur_timestamp = timestamp();
            if(cur_timestamp - last_timestamp >= 30)
            {
                last_timestamp = cur_timestamp;
                if(send(client_sock_fd, CIPHER4, strlen(CIPHER4), 0) == -1)
                {
                    LOG("send heart beat failed: %s", strerror(errno));
                }
                else
                {
                    LOG("send heart beat successfully!");
                    is_wait = 1;
                }
            }
        }
    }
}
