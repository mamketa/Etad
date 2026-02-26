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

// Esport Mode
void mediatek_esport() {
    // PPM policies
    if (file_exists("/proc/ppm/policy_status")) {
        FILE *fp = fopen("/proc/ppm/policy_status", "r");
        if (fp) {
            char line[MAX_LINE_LEN];
            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, PPM_POLICY)) {
                    char policy_cmd[10];
                    snprintf(policy_cmd, sizeof(policy_cmd), "%c 0", line[1]);
                    apply(policy_cmd, "/proc/ppm/policy_status");
                }
            }
            fclose(fp);
        }
    }
    
    // FPSGO Tweaks
    apply("0", "/sys/kernel/fpsgo/fstb/adopt_low_fps");
    apply("1", "/sys/pnpmgr/fpsgo_boost/boost_enable");
    apply("1", "/sys/kernel/fpsgo/fstb/gpu_slowdown_check");
     
    // MTK Power and CCI mode
    apply("1", "/proc/cpufreq/cpufreq_cci_mode");
    apply("3", "/proc/cpufreq/cpufreq_power_mode");
    
    // Perf limiter ON
    apply("1", "/proc/perfmgr/syslimiter/syslimiter_force_disable");
    
    // DDR Boost mode
    apply("1", "/sys/devices/platform/boot_dramboost/dramboost/dramboost");
    
    // EAS/HMP Switch
    apply("0", "/sys/devices/system/cpu/eas/enable");
        
    // GPU Frequency
    if (LITE_MODE == 0) {
        if (file_exists("/proc/gpufreqv2")) {
            apply("0", "/proc/gpufreqv2/fix_target_opp_index");
        } else if (file_exists("/proc/gpufreq/gpufreq_opp_dump")) {
            FILE *fp = fopen("/proc/gpufreq/gpufreq_opp_dump", "r");
            if (fp) {
                char line[MAX_LINE_LEN];
                if (fgets(line, sizeof(line), fp)) {
                    char *freq_str = strstr(line, "freq =");
                    if (freq_str) {
                        long freq = atol(freq_str + 6);
                        apply_ll(freq, "/proc/gpufreq/gpufreq_opp_freq");
                    }
                }
                fclose(fp);
            }
        }
    } else {
        apply("0", "/proc/gpufreq/gpufreq_opp_freq");
        apply("-1", "/proc/gpufreqv2/fix_target_opp_index");
        
        // Set min freq via GED
        int mid_oppfreq;
        if (file_exists("/proc/gpufreqv2/gpu_working_opp_table")) {
            mid_oppfreq = mtk_gpufreq_midfreq_index("/proc/gpufreqv2/gpu_working_opp_table");
        } else if (file_exists("/proc/gpufreq/gpufreq_opp_dump")) {
            mid_oppfreq = mtk_gpufreq_midfreq_index("/proc/gpufreq/gpufreq_opp_dump");
        } else {
            mid_oppfreq = 0;
        }
        
        apply_ll(mid_oppfreq, "/sys/kernel/ged/hal/custom_boost_gpu_freq");
    }
    
    // Disable GPU Power limiter
    apply("ignore_batt_oc 1", "/proc/gpufreq/gpufreq_power_limited");
    apply("ignore_batt_percent 1", "/proc/gpufreq/gpufreq_power_limited");
    apply("ignore_low_batt 1", "/proc/gpufreq/gpufreq_power_limited");
    apply("ignore_thermal_protect 1", "/proc/gpufreq/gpufreq_power_limited");
    apply("ignore_pbm_limited 1", "/proc/gpufreq/gpufreq_power_limited");
    
    // Disable battery current limiter
    apply("stop 1", "/proc/mtk_batoc_throttling/battery_oc_protect_stop");
    
    // DRAM Frequency
    if (LITE_MODE == 0) {
        apply("0", "/sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_req_ddr_opp");
        apply("0", "/sys/kernel/helio-dvfsrc/dvfsrc_force_vcore_dvfs_opp");
        devfreq_max_perf("/sys/class/devfreq/mtk-dvfsrc-devfreq");
    } else {
        apply("-1", "/sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_req_ddr_opp");
        apply("-1", "/sys/kernel/helio-dvfsrc/dvfsrc_force_vcore_dvfs_opp");
        devfreq_mid_perf("/sys/class/devfreq/mtk-dvfsrc-devfreq");
    }
    
    // Eara Thermal
    apply("0", "/sys/kernel/eara_thermal/enable");
}

// Balanced Mode
void mediatek_balanced() {
    // PPM policies
    if (file_exists("/proc/ppm/policy_status")) {
        FILE *fp = fopen("/proc/ppm/policy_status", "r");
        if (fp) {
            char line[MAX_LINE_LEN];
            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, PPM_POLICY)) {
                    char policy_cmd[10];
                    snprintf(policy_cmd, sizeof(policy_cmd), "%c 1", line[1]);
                    apply(policy_cmd, "/proc/ppm/policy_status");
                }
            }
            fclose(fp);
        }
    }
        
    // FPSGO Tweaks
    apply("1", "/sys/kernel/fpsgo/common/force_onoff");
    apply("1", "/sys/kernel/fpsgo/fstb/adopt_low_fps");
    apply("0", "/sys/pnpmgr/fpsgo_boost/boost_enable");
    apply("0", "/sys/kernel/fpsgo/fstb/gpu_slowdown_check");
    apply("1", "/sys/kernel/fpsgo/fstb/fstb_self_ctrl_fps_enable");
    apply("0", "/sys/kernel/fpsgo/fbt/enable_switch_down_throttle");
    apply("1", "/sys/module/mtk_fpsgo/parameters/perfmgr_enable");
    
    // MTK Power and CCI mode
    apply("0", "/proc/cpufreq/cpufreq_cci_mode");
    apply("2", "/proc/cpufreq/cpufreq_power_mode");
    
    // Perf limiter ON
    apply("0", "/proc/perfmgr/syslimiter/syslimiter_force_disable");
    
    // DDR Boost mode
    apply("0", "/sys/devices/platform/boot_dramboost/dramboost/dramboost");
    
    // EAS/HMP Switch
    apply("2", "/sys/devices/system/cpu/eas/enable");
    
    // Disable GED KPI
    apply("0", "/sys/module/ged/parameters/is_GED_KPI_enabled");
    
    // GPU Frequency
    write_file("0", "/proc/gpufreq/gpufreq_opp_freq");
    write_file("-1", "/proc/gpufreqv2/fix_target_opp_index");
    
    // Enable GPU Dynamic Scaling
    apply("1", "/sys/module/ged/parameters/gpu_dvfs_enable");
    
    // Reset min freq via GED
    int min_oppfreq;
    if (file_exists("/proc/gpufreqv2/gpu_working_opp_table")) {
        min_oppfreq = mtk_gpufreq_minfreq_index("/proc/gpufreqv2/gpu_working_opp_table");
    } else if (file_exists("/proc/gpufreq/gpufreq_opp_dump")) {
        min_oppfreq = mtk_gpufreq_minfreq_index("/proc/gpufreq/gpufreq_opp_dump");
    } else {
        min_oppfreq = 0;
    }
    
    apply_ll(min_oppfreq, "/sys/kernel/ged/hal/custom_boost_gpu_freq");
    
    // GPU Power limiter
    apply("ignore_batt_oc 0", "/proc/gpufreq/gpufreq_power_limited");
    apply("ignore_batt_percent 0", "/proc/gpufreq/gpufreq_power_limited");
    apply("ignore_low_batt 0", "/proc/gpufreq/gpufreq_power_limited");
    apply("ignore_thermal_protect 0", "/proc/gpufreq/gpufreq_power_limited");
    apply("ignore_pbm_limited 0", "/proc/gpufreq/gpufreq_power_limited");
    
    // Enable battery current limiter
    apply("stop 0", "/proc/mtk_batoc_throttling/battery_oc_protect_stop");
    
    // DRAM Frequency
    write_file("-1", "/sys/devices/platform/10012000.dvfsrc/helio-dvfsrc/dvfsrc_req_ddr_opp");
    write_file("-1", "/sys/kernel/helio-dvfsrc/dvfsrc_force_vcore_dvfs_opp");
    devfreq_unlock("/sys/class/devfreq/mtk-dvfsrc-devfreq");
    
    // Eara Thermal
    apply("1", "/sys/kernel/eara_thermal/enable");
}

// Efficiency mode
void mediatek_efficiency() {    
    // FPSGO Tweaks
    apply("1", "/sys/kernel/fpsgo/fstb/adopt_low_fps");
    apply("0", "/sys/pnpmgr/fpsgo_boost/boost_enable");
    apply("0", "/sys/kernel/fpsgo/fstb/gpu_slowdown_check");
    apply("1", "/sys/kernel/fpsgo/fbt/enable_switch_down_throttle");
    apply("1", "/sys/module/mtk_fpsgo/parameters/perfmgr_enable");
    
    // Perf limiter ON
    apply("0", "/proc/perfmgr/syslimiter/syslimiter_force_disable");
    
    // DDR Boost mode
    apply("0", "/sys/devices/platform/boot_dramboost/dramboost/dramboost");
    
    // EAS/HMP Switch
    apply("1", "/sys/devices/system/cpu/eas/enable");
    
    // MTK CPU Power mode to low power
    apply("2", "/proc/cpufreq/cpufreq_cci_mode");
    apply("1", "/proc/cpufreq/cpufreq_power_mode");
    
    // GPU Frequency
    if (file_exists("/proc/gpufreqv2")) {
        int min_gpufreq_index = mtk_gpufreq_minfreq_index("/proc/gpufreqv2/gpu_working_opp_table");
        apply_ll(min_gpufreq_index, "/proc/gpufreqv2/fix_target_opp_index");
    } else if (file_exists("/proc/gpufreq/gpufreq_opp_dump")) {
        FILE *fp = fopen("/proc/gpufreq/gpufreq_opp_dump", "r");
        if (fp) {
            char line[MAX_LINE_LEN];
            long min_freq = LONG_MAX;
            
            while (fgets(line, sizeof(line), fp)) {
                char *freq_str = strstr(line, "freq =");
                if (freq_str) {
                    long freq = atol(freq_str + 6);
                    if (freq < min_freq) min_freq = freq;
                }
            }
            fclose(fp);
            
            if (min_freq != LONG_MAX) {
                apply_ll(min_freq, "/proc/gpufreq/gpufreq_opp_freq");
            }
        }
    }
}
