//
// Created by root on 19-6-27.
//
#include "log.h"

extern conf cf;

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void logger(const char *source_ptr, size_t line, const char *format_ptr, ...)
{
    char *dir_ptr = cf.log;
    int is_log_file = strlen(dir_ptr) > 0 ? 1 : 0;
    FILE *fp = NULL;
    char file_path[PATH_MAX] = {0};
    char *buf_ptr = NULL;
    struct tm *tm_ptr = NULL;
    time_t t;

    pthread_mutex_lock(&log_mutex);

    time(&t);
    tm_ptr = localtime(&t);

    if(is_log_file)
    {
        if(make_dir(dir_ptr) != 0)
        {
            fprintf(stderr, "make dirs failed!\n");
            return;
        }

        sprintf(file_path, "%s/%04d-%02d-%02d.log",
                dir_ptr,
                tm_ptr->tm_year + 1900,
                tm_ptr->tm_mon + 1,
                tm_ptr->tm_mday);

        if(!(fp = fopen(file_path, "a+")))
        {
            pthread_mutex_unlock(&log_mutex);
            return;
        }
    }
    else
    {
        fp = stdout;
    }

    va_list list;
    va_start(list, format_ptr);
    vasprintf(&buf_ptr, format_ptr, list);
    va_end(list);

    if(!buf_ptr)
    {
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    fprintf(fp, "[%02d:%02d:%02d %s %ld] %s\n",
            tm_ptr->tm_hour,
            tm_ptr->tm_min,
            tm_ptr->tm_sec,
            source_ptr,
            line,
            buf_ptr);

    free(buf_ptr);
    fsync(fileno(fp));
    if(is_log_file)
    {
        fclose(fp);
    }

    pthread_mutex_unlock(&log_mutex);
}
