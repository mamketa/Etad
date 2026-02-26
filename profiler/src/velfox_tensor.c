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

#include "velfox_soc.h"
#include "velfox_cpufreq.h"
#include "velfox_devfreq.h"

// Esport mode
void tensor_esport() {
    // Find GPU path
    DIR *dir = opendir("/sys/devices/platform");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".mali")) {
                char gpu_path[MAX_PATH_LEN];
                snprintf(gpu_path, sizeof(gpu_path), "/sys/devices/platform/%s", ent->d_name);
                
                char avail_path[MAX_PATH_LEN];
                snprintf(avail_path, sizeof(avail_path), "%s/available_frequencies", gpu_path);
                
                if (file_exists(avail_path)) {
                    long max_freq = get_max_freq(avail_path);
                    
                    char max_freq_path[MAX_PATH_LEN];
                    snprintf(max_freq_path, sizeof(max_freq_path), "%s/scaling_max_freq", gpu_path);
                    apply_ll(max_freq, max_freq_path);
                    
                    if (LITE_MODE == 1) {
                        long mid_freq = get_mid_freq(avail_path);
                        char min_freq_path[MAX_PATH_LEN];
                        snprintf(min_freq_path, sizeof(min_freq_path), "%s/scaling_min_freq", gpu_path);
                        apply_ll(mid_freq, min_freq_path);
                    } else {
                        char min_freq_path[MAX_PATH_LEN];
                        snprintf(min_freq_path, sizeof(min_freq_path), "%s/scaling_min_freq", gpu_path);
                        apply_ll(max_freq, min_freq_path);
                    }
                }
                break;
            }
        }
        closedir(dir);
    }
    
    // DRAM frequency
    if (DEVICE_MITIGATION == 0) {
        DIR *dir = opendir("/sys/class/devfreq");
        if (dir) {
            struct dirent *ent;
            while ((ent = readdir(dir)) != NULL) {
                if (strstr(ent->d_name, "devfreq_mif")) {
                    char path[MAX_PATH_LEN];
                    snprintf(path, sizeof(path), "/sys/class/devfreq/%s", ent->d_name);
                    
                    if (LITE_MODE == 1) {
                        devfreq_mid_perf(path);
                    } else {
                        devfreq_max_perf(path);
                    }
                }
            }
            closedir(dir);
        }
    }
}

// Adaptive mode
void tensor_adaptive() {
    // Find GPU path
    DIR *dir = opendir("/sys/devices/platform");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".mali")) {
                char gpu_path[MAX_PATH_LEN];
                snprintf(gpu_path, sizeof(gpu_path), "/sys/devices/platform/%s", ent->d_name);
                
                char avail_path[MAX_PATH_LEN];
                snprintf(avail_path, sizeof(avail_path), "%s/available_frequencies", gpu_path);
                
                if (file_exists(avail_path)) {
                    long max_freq = get_max_freq(avail_path);
                    long min_freq = get_min_freq(avail_path);
                    
                    char max_freq_path[MAX_PATH_LEN];
                    snprintf(max_freq_path, sizeof(max_freq_path), "%s/scaling_max_freq", gpu_path);
                    write_ll(max_freq, max_freq_path);
                    
                    char min_freq_path[MAX_PATH_LEN];
                    snprintf(min_freq_path, sizeof(min_freq_path), "%s/scaling_min_freq", gpu_path);
                    write_ll(min_freq, min_freq_path);
                }
                break;
            }
        }
        closedir(dir);
    }
    
    // DRAM frequency
    if (DEVICE_MITIGATION == 0) {
        DIR *dir = opendir("/sys/class/devfreq");
        if (dir) {
            struct dirent *ent;
            while ((ent = readdir(dir)) != NULL) {
                if (strstr(ent->d_name, "devfreq_mif")) {
                    char path[MAX_PATH_LEN];
                    snprintf(path, sizeof(path), "/sys/class/devfreq/%s", ent->d_name);
                    devfreq_unlock(path);
                }
            }
            closedir(dir);
        }
    }
}

// Efficiency mode
void tensor_efficiency() {
    // Find GPU path
    DIR *dir = opendir("/sys/devices/platform");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".mali")) {
                char gpu_path[MAX_PATH_LEN];
                snprintf(gpu_path, sizeof(gpu_path), "/sys/devices/platform/%s", ent->d_name);
                
                char avail_path[MAX_PATH_LEN];
                snprintf(avail_path, sizeof(avail_path), "%s/available_frequencies", gpu_path);
                
                if (file_exists(avail_path)) {
                    long freq = get_min_freq(avail_path);
                    
                    char min_freq_path[MAX_PATH_LEN];
                    snprintf(min_freq_path, sizeof(min_freq_path), "%s/scaling_min_freq", gpu_path);
                    apply_ll(freq, min_freq_path);
                    
                    char max_freq_path[MAX_PATH_LEN];
                    snprintf(max_freq_path, sizeof(max_freq_path), "%s/scaling_max_freq", gpu_path);
                    apply_ll(freq, max_freq_path);
                }
                break;
            }
        }
        closedir(dir);
    }
}
