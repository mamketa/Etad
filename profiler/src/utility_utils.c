/*
 * Copyright (C) 2025-2026 VelocityFox22
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "velfox_common.h"

int file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

int read_int_from_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    int value;
    fscanf(fp, "%d", &value);
    fclose(fp);
    return value;
}

void read_string_from_file(char *buffer, size_t size, const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        buffer[0] = '\0';
        return;
    }
    fgets(buffer, size, fp);
    buffer[strcspn(buffer, "\n")] = '\0';
    fclose(fp);
}

int apply(const char *value, const char *path) {
    if (!path || !value) return 0;
    /* Check if the file exists */
    if (access(path, F_OK) != 0)
        return 0;
    FILE *fp;
    char buf[256] = {0};
    fp = fopen(path, "r");
    if (fp) {
        if (fgets(buf, sizeof(buf), fp)) {
            buf[strcspn(buf, "\n")] = '\0';
            /* If it's the same, skip writing  */
            if (strcmp(buf, value) == 0) {
                fclose(fp);
                return 1;
            }
        }
        fclose(fp);
    }
     /* If it fails, then try changing the permissions */
    fp = fopen(path, "w");
    if (!fp) {
        chmod(path, 0644);
        fp = fopen(path, "w");
        if (!fp) {
            chmod(path, 0444);
            return 0;
        }
    }
    int ret = fprintf(fp, "%s", value);
    fflush(fp);
    fclose(fp);
    chmod(path, 0444);
    if (ret < 0)
        return 0;

    return 1;
}

int write_file(const char *value, const char *path) {
    if (!file_exists(path)) return 0;    
    chmod(path, 0644);
    FILE *fp = fopen(path, "w");
    if (!fp) {
        chmod(path, 0444);
        return 0;
    }
    fprintf(fp, "%s", value);
    fclose(fp);
    chmod(path, 0444);
    return 1;
}

int apply_ll(long long value, const char *path) {
    char str[50];
    snprintf(str, sizeof(str), "%lld", value);
    return apply(str, path);
}

int write_ll(long long value, const char *path) {
    char str[50];
    snprintf(str, sizeof(str), "%lld", value);
    return write_file(str, path);
}
