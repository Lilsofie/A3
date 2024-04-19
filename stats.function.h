#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <utmp.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#ifndef STATS_FUNCTIONS_H
#define STATS_FUNCTIONS_H

#define MAX_LENGTH 1024
#define MAX_USERS 100
#define FLAGS_LENGTH 5
#define GB_CONVERTER 1024 * 1024 * 1024

void moveCursorTo(int row, int col);
void clearScreen();
void print_system_status(int flags[FLAGS_LENGTH], int samples, int tdelay);

#endif