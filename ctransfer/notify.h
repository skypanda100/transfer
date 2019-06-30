//
// Created by root on 19-6-30.
//

#ifndef CTRANSFER_NOTIFY_H
#define CTRANSFER_NOTIFY_H

#include <sys/inotify.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <regex.h>
#include "def.h"
#include "log.h"
#include "util.h"

void watch();

#endif //CTRANSFER_NOTIFY_H
