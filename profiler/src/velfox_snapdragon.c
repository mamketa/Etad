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

// Apex mode
void snapdragon_apex() {
    // Qualcomm CPU Bus and DRAM frequencies
    if (DEVICE_MITIGATION == 0) {
        DIR *dir = opendir("/sys/class/devfreq");
        if (dir) {
            struct dirent *ent;
            while ((ent = readdir(dir)) != NULL) {
                char *name = ent->d_name;
                if (strstr(name, "cpu-lat") || strstr(name, "cpu-bw") || 
                    strstr(name, "llccbw") || strstr(name, "bus_llcc") ||
                    strstr(name, "bus_ddr") || strstr(name, "memlat") ||
                    strstr(name, "cpubw") || strstr(name, "kgsl-ddr-qos")) {
                    char path[MAX_PATH_LEN];
                    snprintf(path, sizeof(path), "/sys/class/devfreq/%s", name);
                    if (LITE_MODE == 1) {
                        devfreq_mid_perf(path);
                    } else {
                        devfreq_max_perf(path);
                    }
                }
            }
            closedir(dir);
        }
        
        // Process bus_dcvs components
        const char *components[] = {"DDR", "LLCC", "L3"};
        for (int i = 0; i < 3; i++) {
            char path[MAX_PATH_LEN];
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/bus_dcvs/%s", components[i]);
            
            if (LITE_MODE == 1) {
                qcom_cpudcvs_mid_perf(path);
            } else {
                qcom_cpudcvs_max_perf(path);
            }
        }
    }
    
    // GPU tweak
    const char *gpu_path = "/sys/class/kgsl/kgsl-3d0/devfreq";
    if (LITE_MODE == 0) {
        devfreq_max_perf(gpu_path);
    } else {
        devfreq_mid_perf(gpu_path);
    }
    
    // Force GPU clock on/off
    apply("1", "/sys/class/kgsl/kgsl-3d0/force_clk_on");
    
    // GPU Idle State Control
    apply("1", "/sys/class/kgsl/kgsl-3d0/force_no_nap");
    
    // Disable GPU Bus split
    apply("0", "/sys/class/kgsl/kgsl-3d0/bus_split");
        
    // Disable GPU Performance Counters
    apply("0", "/sys/class/kgsl/kgsl-3d0/perfcounter");

    // Enable Adreno Boost
    apply("1", "/sys/class/kgsl/kgsl-3d0/devfreq/adrenoboost");
}

// Adaptive mode
void snapdragon_adaptive() {
    // Qualcomm CPU Bus and DRAM frequencies
    if (DEVICE_MITIGATION == 0) {
        DIR *dir = opendir("/sys/class/devfreq");
        if (dir) {
            struct dirent *ent;
            while ((ent = readdir(dir)) != NULL) {
                char *name = ent->d_name;
                if (strstr(name, "cpu-lat") || strstr(name, "cpu-bw") || 
                    strstr(name, "llccbw") || strstr(name, "bus_llcc") ||
                    strstr(name, "bus_ddr") || strstr(name, "memlat") ||
                    strstr(name, "cpubw") || strstr(name, "kgsl-ddr-qos")) {
                    
                    char path[MAX_PATH_LEN];
                    snprintf(path, sizeof(path), "/sys/class/devfreq/%s", name);
                    devfreq_unlock(path);
                }
            }
            closedir(dir);
        }
        
        // Process bus_dcvs components
        const char *components[] = {"DDR", "LLCC", "L3"};
        for (int i = 0; i < 3; i++) {
            char path[MAX_PATH_LEN];
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/bus_dcvs/%s", components[i]);
            qcom_cpudcvs_unlock(path);
        }
    }
    
    // Revert GPU tweak
    devfreq_unlock("/sys/class/kgsl/kgsl-3d0/devfreq");
    
    // Free GPU clock on/off
    apply("0", "/sys/class/kgsl/kgsl-3d0/force_clk_on");
    
    // GPU Idle State Control
    apply("0", "/sys/class/kgsl/kgsl-3d0/force_no_nap");
    
    // Enable back GPU Bus split
    apply("1", "/sys/class/kgsl/kgsl-3d0/bus_split");
    
    // Disable GPU Performance Counters
    apply("0", "/sys/class/kgsl/kgsl-3d0/perfcounter");

    // Disable Adreno Boost
    apply("0", "/sys/class/kgsl/kgsl-3d0/devfreq/adrenoboost");
}

// Efficiency mode
void snapdragon_efficiency() {
    // GPU Frequency
    devfreq_min_perf("/sys/class/kgsl/kgsl-3d0/devfreq");
    
    // Free GPU clock on/off
    apply("0", "/sys/class/kgsl/kgsl-3d0/force_clk_on");
    
    // GPU Idle State Control
    apply("0", "/sys/class/kgsl/kgsl-3d0/force_no_nap");
    
    // Enable back GPU Bus split
    apply("1", "/sys/class/kgsl/kgsl-3d0/bus_split");
        
    // Disable GPU Performance Counters
    apply("0", "/sys/class/kgsl/kgsl-3d0/perfcounter");

    // Disable Adreno Boost
    apply("0", "/sys/class/kgsl/kgsl-3d0/devfreq/adrenoboost");
}
