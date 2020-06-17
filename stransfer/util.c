//
// Created by zhengdongtian on 19-6-27.
//

#include "util.h"
#include <sys/time.h>

char *l_trim(char *output_ptr, const char *input_ptr)
{
    assert(input_ptr != NULL);
    assert(output_ptr != NULL);
    assert(output_ptr != input_ptr);
    for(;*input_ptr != '\0' && isspace(*input_ptr);++input_ptr)
    {
        ;
    }
    return strcpy(output_ptr, input_ptr);
}

char *a_trim(char *output_ptr, const char *input_ptr)
{
    char *p = NULL;
    assert(input_ptr != NULL);
    assert(output_ptr != NULL);
    l_trim(output_ptr, input_ptr);
    for(p = output_ptr + strlen(output_ptr) - 1;p >= output_ptr && isspace(*p);--p)
    {
        ;
    }
    *(++p) = '\0';
    return output_ptr;
}

int make_dir(const char *path_ptr)
{
    char str[PATH_MAX] = {0};
    strcpy(str, path_ptr);
    int len = strlen(str);

    for(int i = 0;i < len;i++)
    {
        if(str[i] == '/')
        {
            str[i] = '\0';
            if(strlen(str) == 0)
            {
                str[i] = '/';
                continue;
            }
            if(access(str, F_OK) != 0)
            {
                if(mkdir(str, 0777) != 0)
                {
                    return -1;
                }
            }
            str[i] = '/';
        }
    }

    if(len > 0 && access(str, F_OK) != 0)
    {
        if(mkdir(str, 0777) != 0)
        {
            return -1;
        }
    }

    return 0;
}

long timestamp()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);

    return tv.tv_sec;
}