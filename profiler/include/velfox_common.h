#ifndef VELFOX_COMMON_H
#define VELFOX_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#define MODULE_CONFIG "/data/adb/.config/Velfox"
#define MAX_PATH_LEN 256
#define MAX_LINE_LEN 1024
#define MAX_OPP_COUNT 50

// Global variables
extern int SOC;
extern int LITE_MODE;
extern int DEVICE_MITIGATION;
extern char DEFAULT_CPU_GOV[50];
extern char PPM_POLICY[512];

// Utility function prototypes
int file_exists(const char *path);
int read_int_from_file(const char *path);
void read_string_from_file(char *buffer, size_t size, const char *path);
int apply(const char *value, const char *path);
int write_file(const char *value, const char *path);
int apply_ll(long long value, const char *path);
int write_ll(long long value, const char *path);

#endif // VELFOX_COMMON_H
