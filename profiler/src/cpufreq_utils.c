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
#include "velfox_cpufreq.h"

// Change governor
void change_cpu_gov(const char *gov) {
    DIR *dir;
    struct dirent *ent;
    char path[MAX_PATH_LEN];
    // Change governor for each CPU
    for (int i = 0; i <= 15; i++) {
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", i);
        if (file_exists(path)) {
            chmod(path, 0644);
            write_file(gov, path);
        }
    }    
    // Also update policy governors
    dir = opendir("/sys/devices/system/cpu/cpufreq");
    if (dir) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, "policy")) {
                snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpufreq/%s/scaling_governor", ent->d_name);
                if (file_exists(path)) {
                    chmod(path, 0444);
                }
            }
        }
        closedir(dir);
    }
}

// Frequency calculation functions
long get_max_freq(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    long max_freq = 0;
    long freq;
    while (fscanf(fp, "%ld", &freq) == 1) {
        if (freq > max_freq) max_freq = freq;
    }
    fclose(fp);
    return max_freq;
}

long get_min_freq(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    long min_freq = LONG_MAX;
    long freq;
    while (fscanf(fp, "%ld", &freq) == 1) {
        if (freq > 0 && freq < min_freq) min_freq = freq;
    }
    fclose(fp);
    return (min_freq == LONG_MAX) ? 0 : min_freq;
}

long get_mid_freq(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    long freqs[MAX_OPP_COUNT];
    int count = 0;
    while (fscanf(fp, "%ld", &freqs[count]) == 1 && count < MAX_OPP_COUNT - 1) {
        count++;
    }
    fclose(fp);
    if (count == 0) return 0;
    // Sort frequencies
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (freqs[j] > freqs[j + 1]) {
                long temp = freqs[j];
                freqs[j] = freqs[j + 1];
                freqs[j + 1] = temp;
            }
        }
    }
    // Get middle frequency
    return freqs[count / 2];
}

// CPU frequency settings
void cpufreq_ppm_max_perf() {
    DIR *dir;
    struct dirent *ent;
    char path[MAX_PATH_LEN];
    int cluster = -1;
    dir = opendir("/sys/devices/system/cpu/cpufreq");
    if (!dir) return;
    while ((ent = readdir(dir)) != NULL) {
        if (strstr(ent->d_name, "policy")) {
            cluster++;
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpufreq/%s/cpuinfo_max_freq", ent->d_name);
            long cpu_maxfreq = read_int_from_file(path);
            char ppm_cmd[100];
            snprintf(ppm_cmd, sizeof(ppm_cmd), "%d %ld", cluster, cpu_maxfreq);
            apply(ppm_cmd, "/proc/ppm/policy/hard_userlimit_max_cpu_freq");
            if (LITE_MODE == 1) {
                snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpufreq/%s/scaling_available_frequencies", ent->d_name);
                long cpu_midfreq = get_mid_freq(path);
                snprintf(ppm_cmd, sizeof(ppm_cmd), "%d %ld", cluster, cpu_midfreq);
                apply(ppm_cmd, "/proc/ppm/policy/hard_userlimit_min_cpu_freq");
            } else {
                snprintf(ppm_cmd, sizeof(ppm_cmd), "%d %ld", cluster, cpu_maxfreq);
                apply(ppm_cmd, "/proc/ppm/policy/hard_userlimit_min_cpu_freq");
            }
        }
    }
    closedir(dir);
}

void cpufreq_max_perf() {
    for (int i = 0; i <= 15; i++) {
        char path[MAX_PATH_LEN];
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", i);
        if (!file_exists(path)) continue;
        long cpu_maxfreq = read_int_from_file(path);
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", i);
        apply_ll(cpu_maxfreq, path);
        if (LITE_MODE == 1) {
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies", i);
            long cpu_midfreq = get_mid_freq(path);
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", i);
            apply_ll(cpu_midfreq, path);
        } else {
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", i);
            apply_ll(cpu_maxfreq, path);
        }
    }
    // Set permissions
    DIR *dir = opendir("/sys/devices/system/cpu/cpufreq");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, "policy")) {
                char policy_path[MAX_PATH_LEN];
                snprintf(policy_path, sizeof(policy_path), "/sys/devices/system/cpu/cpufreq/%s/scaling_max_freq", ent->d_name);
                chmod(policy_path, 0444);
                
                snprintf(policy_path, sizeof(policy_path), "/sys/devices/system/cpu/cpufreq/%s/scaling_min_freq", ent->d_name);
                chmod(policy_path, 0444);
            }
        }
        closedir(dir);
    }
}

void cpufreq_ppm_unlock() {
    DIR *dir;
    struct dirent *ent;
    char path[MAX_PATH_LEN];
    int cluster = 0; 
    dir = opendir("/sys/devices/system/cpu/cpufreq");
    if (!dir) return;
    while ((ent = readdir(dir)) != NULL) {
        if (strstr(ent->d_name, "policy")) {
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpufreq/%s/cpuinfo_max_freq", ent->d_name);
            long cpu_maxfreq = read_int_from_file(path);
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpufreq/%s/cpuinfo_min_freq", ent->d_name);
            long cpu_minfreq = read_int_from_file(path);
            char ppm_cmd[100];
            snprintf(ppm_cmd, sizeof(ppm_cmd), "%d %ld", cluster, cpu_maxfreq);
            write_file(ppm_cmd, "/proc/ppm/policy/hard_userlimit_max_cpu_freq");
            snprintf(ppm_cmd, sizeof(ppm_cmd), "%d %ld", cluster, cpu_minfreq);
            write_file(ppm_cmd, "/proc/ppm/policy/hard_userlimit_min_cpu_freq");
            
            cluster++;
        }
    }
    closedir(dir);
}

void cpufreq_unlock() {
    for (int i = 0; i <= 15; i++) {
        char path[MAX_PATH_LEN];
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", i);
        if (!file_exists(path)) continue;
        long cpu_maxfreq = read_int_from_file(path);
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", i);
        long cpu_minfreq = read_int_from_file(path);
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", i);
        write_ll(cpu_maxfreq, path);
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", i);
        write_ll(cpu_minfreq, path);
    }
    // Set permissions
    DIR *dir = opendir("/sys/devices/system/cpu/cpufreq");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, "policy")) {
                char policy_path[MAX_PATH_LEN];
                snprintf(policy_path, sizeof(policy_path), "/sys/devices/system/cpu/cpufreq/%s/scaling_max_freq", ent->d_name);
                chmod(policy_path, 0644);
                
                snprintf(policy_path, sizeof(policy_path), "/sys/devices/system/cpu/cpufreq/%s/scaling_min_freq", ent->d_name);
                chmod(policy_path, 0644);
            }
        }
        closedir(dir);
    }
}
