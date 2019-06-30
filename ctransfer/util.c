//
// Created by root on 19-6-30.
//

#include "util.h"

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


void list_dir(const char *path_ptr, char ***dir_ptr_ptr_ptr, size_t *dirs_len_ptr)
{
    DIR *d_ptr;
    struct dirent *file_ptr;
    struct stat st;
    char full_path[PATH_MAX] = {0};

    if(!(d_ptr = opendir(path_ptr)))
    {
        return;
    }

    while((file_ptr = readdir(d_ptr)) != NULL)
    {
        if(strncmp(file_ptr->d_name, ".", 1) == 0)
        {
            continue;
        }

        memset(full_path, 0, sizeof(full_path) / sizeof(char));
        sprintf(full_path, "%s/%s", path_ptr, file_ptr->d_name);

        if(stat(full_path, &st) >= 0 && S_ISDIR(st.st_mode))
        {
            *dirs_len_ptr += 1;
            *dir_ptr_ptr_ptr = realloc(*dir_ptr_ptr_ptr, sizeof(char *) * (*dirs_len_ptr));
            (*dir_ptr_ptr_ptr)[*dirs_len_ptr - 1] = strdup(full_path);
            list_dir(full_path, dir_ptr_ptr_ptr, dirs_len_ptr);
        }
    }
    closedir(d_ptr);
}