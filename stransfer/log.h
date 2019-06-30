//
// Created by zhengdongtian on 19-6-27.
//

#ifndef STRANSFER_LOG_H
#define STRANSFER_LOG_H
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>
#include <sys/stat.h>
#include "config.h"
#include "util.h"

#define LOG(fmt, args...) logger(__FILE__, __LINE__, fmt, ##args);

void logger(const char *source_ptr, size_t line, const char *format_ptr, ...);

#endif //STRANSFER_LOG_H
