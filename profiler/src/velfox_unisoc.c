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
void unisoc_esport() {
    // Find GPU path
    DIR *dir = opendir("/sys/class/devfreq");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".gpu")) {
                char gpu_path[MAX_PATH_LEN];
                snprintf(gpu_path, sizeof(gpu_path), "/sys/class/devfreq/%s", ent->d_name);
                
                if (LITE_MODE == 0) {
                    devfreq_max_perf(gpu_path);
                } else {
                    devfreq_mid_perf(gpu_path);
                }
                break;
            }
        }
        closedir(dir);
    }
}

// Adaptive mode
void unisoc_adaptive() {
    // GPU Frequency
    DIR *dir = opendir("/sys/class/devfreq");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".gpu")) {
                char gpu_path[MAX_PATH_LEN];
                snprintf(gpu_path, sizeof(gpu_path), "/sys/class/devfreq/%s", ent->d_name);
                devfreq_unlock(gpu_path);
                break;
            }
        }
        closedir(dir);
    }
}

// Efficiency mode 
void unisoc_efficiency() {
    // GPU Frequency
    DIR *dir = opendir("/sys/class/devfreq");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".gpu")) {
                char gpu_path[MAX_PATH_LEN];
                snprintf(gpu_path, sizeof(gpu_path), "/sys/class/devfreq/%s", ent->d_name);
                devfreq_min_perf(gpu_path);
                break;
            }
        }
        closedir(dir);
    }
}
