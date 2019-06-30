//
// Created by root on 19-6-30.
//

#ifndef CTRANSFER_UTIL_H
#define CTRANSFER_UTIL_H
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

char *l_trim(char *output_ptr, const char *input_ptr);
char *a_trim(char *output_ptr, const char *input_ptr);
int make_dir(const char *path_ptr);
void list_dir(const char *path_ptr, char ***dir_ptr_ptr_ptr, size_t *dirs_len_ptr);

#endif //CTRANSFER_UTIL_H
