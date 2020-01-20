//
// Created by root on 19-6-30.
//
#include "config.h"

extern conf cf;

static char *key_host_ptr = "host";
static char *key_port_ptr = "port";
static char *key_key_ptr = "key";
static char *key_log_ptr = "log";
static char *key_ignore_ptr = "ignore";
static char *key_src_dir_ptr = "src_dir";
static char *key_dst_dir_ptr = "dst_dir";
static char *key_buffer_size_ptr = "buffer_size";

void config(const char *conf_path_ptr)
{
    char key[32] = {0};
    char val_host[32] = {0};
    char val_port[10] = {0};
    char val_key[128] = {0};
    char val_log[1024] = {0};
    char val_ignore[128] = {0};
    char val_src_dir[1024] = {0};
    char val_dst_dir[1024] = {0};
    char val_buffer_size[10] = {0};
    char *buf, *c;
    char buf_i[1024], buf_o[1024];
    FILE *fp;
    if((fp=fopen(conf_path_ptr, "r")) == NULL)
    {
        fprintf(stderr, "openfile [%s] error [%s]\n", conf_path_ptr, strerror(errno));
        exit(1);
    }
    fseek(fp, 0, SEEK_SET);

    while(!feof(fp) && fgets(buf_i, 1024, fp) != NULL)
    {
        l_trim(buf_o, buf_i);
        if(strlen(buf_o) <= 0)
            continue;
        buf = buf_o;

        if(buf[0] == '#')
        {
            continue;
        }
        else
        {
            if((c = strchr(buf, '=')) == NULL)
                continue;
            memset(key, 0, sizeof(key));
            sscanf(buf, "%[^= \t]", key);
            if(strcmp(key, key_host_ptr) == 0)
            {
                sscanf(++c, "%[^\n\r]", val_host);
                char *val_o = (char *)malloc(strlen(val_host) + 1);
                if(val_o != NULL)
                {
                    memset(val_o, 0, strlen(val_host) + 1);
                    a_trim(val_o, val_host);
                    if(val_o && strlen(val_o) > 0)
                        strcpy(val_host, val_o);
                    free(val_o);
                    val_o = NULL;
                }
            }
            else if(strcmp(key, key_port_ptr) == 0)
            {
                sscanf(++c, "%[^\n\r]", val_port);
                char *val_o = (char *)malloc(strlen(val_port) + 1);
                if(val_o != NULL)
                {
                    memset(val_o, 0, strlen(val_port) + 1);
                    a_trim(val_o, val_port);
                    if(val_o && strlen(val_o) > 0)
                        strcpy(val_port, val_o);
                    free(val_o);
                    val_o = NULL;
                }
            }
            else if(strcmp(key, key_key_ptr) == 0)
            {
                sscanf(++c, "%[^\n\r]", val_key);
                char *val_o = (char *)malloc(strlen(val_key) + 1);
                if(val_o != NULL)
                {
                    memset(val_o, 0, strlen(val_key) + 1);
                    a_trim(val_o, val_key);
                    if(val_o && strlen(val_o) > 0)
                        strcpy(val_key, val_o);
                    free(val_o);
                    val_o = NULL;
                }
            }
            else if(strcmp(key, key_log_ptr) == 0)
            {
                sscanf(++c, "%[^\n\r]", val_log);
                char *val_o = (char *)malloc(strlen(val_log) + 1);
                if(val_o != NULL)
                {
                    memset(val_o, 0, strlen(val_log) + 1);
                    a_trim(val_o, val_log);
                    if(val_o && strlen(val_o) > 0)
                        strcpy(val_log, val_o);
                    free(val_o);
                    val_o = NULL;
                }
            }
            else if(strcmp(key, key_ignore_ptr) == 0)
            {
                sscanf(++c, "%[^\n\r]", val_ignore);
                char *val_o = (char *)malloc(strlen(val_ignore) + 1);
                if(val_o != NULL)
                {
                    memset(val_o, 0, strlen(val_ignore) + 1);
                    a_trim(val_o, val_ignore);
                    if(val_o && strlen(val_o) > 0)
                        strcpy(val_ignore, val_o);
                    free(val_o);
                    val_o = NULL;
                }
            }
            else if(strcmp(key, key_src_dir_ptr) == 0)
            {
                sscanf(++c, "%[^\n\r]", val_src_dir);
                char *val_o = (char *)malloc(strlen(val_src_dir) + 1);
                if(val_o != NULL)
                {
                    memset(val_o, 0, strlen(val_src_dir) + 1);
                    a_trim(val_o, val_src_dir);
                    if(val_o && strlen(val_o) > 0)
                        strcpy(val_src_dir, val_o);
                    free(val_o);
                    val_o = NULL;
                }
            }
            else if(strcmp(key, key_dst_dir_ptr) == 0)
            {
                sscanf(++c, "%[^\n\r]", val_dst_dir);
                char *val_o = (char *)malloc(strlen(val_dst_dir) + 1);
                if(val_o != NULL)
                {
                    memset(val_o, 0, strlen(val_dst_dir) + 1);
                    a_trim(val_o, val_dst_dir);
                    if(val_o && strlen(val_o) > 0)
                        strcpy(val_dst_dir, val_o);
                    free(val_o);
                    val_o = NULL;
                }
            }
            else if(strcmp(key, key_buffer_size_ptr) == 0)
            {
                sscanf(++c, "%[^\n\r]", val_buffer_size);
                char *val_o = (char *)malloc(strlen(val_buffer_size) + 1);
                if(val_o != NULL)
                {
                    memset(val_o, 0, strlen(val_buffer_size) + 1);
                    a_trim(val_o, val_buffer_size);
                    if(val_o && strlen(val_o) > 0)
                        strcpy(val_buffer_size, val_o);
                    free(val_o);
                    val_o = NULL;
                }
            }
        }
    }
    fclose(fp);

    if(strlen(val_host) == 0)
    {
        fprintf(stderr, "host can not be empty!\n");
        exit(-1);
    }

    if(strlen(val_port) == 0)
    {
        fprintf(stderr, "port can not be empty!\n");
        exit(-1);
    }

    if(atoi(val_port) < 1 || atoi(val_port) > 65535)
    {
        fprintf(stderr, "port must be 1 ~ 65535!\n");
        exit(-1);
    }

    if(strlen(val_key) == 0)
    {
        fprintf(stderr, "key can not be empty!\n");
        exit(-1);
    }

    if(strlen(val_src_dir) == 0)
    {
        fprintf(stderr, "src dir can not be empty!\n");
        exit(-1);
    }

    if(strlen(val_dst_dir) == 0)
    {
        fprintf(stderr, "dst dir can not be empty!\n");
        exit(-1);
    }

    if(strlen(val_buffer_size) == 0)
    {
        fprintf(stderr, "buffer size can not be empty!\n");
        exit(-1);
    }

    if(atoi(val_buffer_size) < 1 || atoi(val_buffer_size) > BUFFER_SIZE)
    {
        fprintf(stderr, "buffer size must be 1 ~ %d!\n", BUFFER_SIZE);
        exit(-1);
    }

    memset(&cf, 0, sizeof(conf));
    strcpy(cf.host, val_host);
    cf.port = atoi(val_port);
    strcpy(cf.key, val_key);
    strcpy(cf.log, val_log);
    strcpy(cf.ignore, val_ignore);
    strcpy(cf.src_dir, val_src_dir);
    strcpy(cf.dst_dir, val_dst_dir);
    cf.buffer_size = atoi(val_buffer_size);
}
