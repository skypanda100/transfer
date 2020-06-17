//
// Created by zhengdongtian on 19-6-27.
//

#ifndef STRANSFER_UTIL_H
#define STRANSFER_UTIL_H

#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

char *l_trim(char *output_ptr, const char *input_ptr);
char *a_trim(char *output_ptr, const char *input_ptr);
int make_dir(const char *path_ptr);
long timestamp();

#endif //STRANSFER_UTIL_H
