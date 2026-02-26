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
    FILE *fp = fopen(path, "r");
    if (fp) {
        fclose(fp);
        return 1;
    }
    return 0;
}

int read_int_from_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    long long value = 0;
    if (fscanf(fp, "%lld", &value) != 1)
        value = 0;
    fclose(fp);
    return (int)value;
}

void read_string_from_file(char *buffer, size_t size, const char *path) {
    if (!buffer || size == 0) return;
    FILE *fp = fopen(path, "r");
    if (!fp) {
        buffer[0] = '\0';
        return;
    }
    if (fgets(buffer, size, fp) == NULL) {
        buffer[0] = '\0';
    } else {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
    fclose(fp);
}

int apply(const char *value, const char *path) {
    if (!value || !path)
        return 0;
    char current[APPLY_BUF];
    FILE *fp = fopen(path, "r");
    if (fp) {
        if (fgets(current, sizeof(current), fp)) {
            current[strcspn(current, "\n")] = '\0';
            if (strcmp(current, value) == 0) {
                fclose(fp);
                return 1;  // already same → skip write
            }
        }
        fclose(fp);
    }
    fp = fopen(path, "w");
    if (!fp)
        return 0;  // path not exist / not writable → silent skip
    fprintf(fp, "%s", value);
    fclose(fp);
    return 1;
}

int write_file(const char *value, const char *path) {
    if (!value || !path)
        return 0;

    FILE *fp = fopen(path, "w");
    if (!fp)
        return 0;

    fprintf(fp, "%s", value);
    fclose(fp);

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
