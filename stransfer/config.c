//
// Created by zhengdongtian on 19-6-27.
//
#include "config.h"

extern conf cf;

static char *key_host_ptr = "host";
static char *key_port_ptr = "port";
static char *key_key_ptr = "key";
static char *key_log_ptr = "log";

void config(const char *conf_path_ptr)
{
    char key[32] = {0};
    char val_host[32] = {0};
    char val_port[10] = {0};
    char val_key[128] = {0};
    char val_log[1024] = {0};
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
            sscanf(buf, "%[^=|^ |^\t]", key);
            if(strcmp(key, key_host_ptr) == 0)
            {
                sscanf(++c, "%[^\n|^\r]", val_host);
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
                sscanf(++c, "%[^\n|^\r]", val_port);
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
                sscanf(++c, "%[^\n|^\r]", val_key);
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
                sscanf(++c, "%[^\n|^\r]", val_log);
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

    memset(&cf, 0, sizeof(conf));
    strcpy(cf.host, val_host);
    cf.port = atoi(val_port);
    strcpy(cf.key, val_key);
    strcpy(cf.log, val_log);
}
