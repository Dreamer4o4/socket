#ifndef _LOG_
#define _LOG_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#define LOG

int log_init();
int log_write(const char *formate, ...);

#endif