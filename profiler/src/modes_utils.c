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

#include "velfox_modes.h"
#include "velfox_cpufreq.h"
#include "velfox_devfreq.h"
#include "velfox_soc.h"

// Global variables definition
int SOC = 0;
int LITE_MODE = 0;
int DEVICE_MITIGATION = 0;
char DEFAULT_CPU_GOV[50] = "schedutil";
char PPM_POLICY[512] = "";

void read_configs() {
    char soc_path[MAX_PATH_LEN];
    snprintf(soc_path, sizeof(soc_path), "%s/soc_recognition", MODULE_CONFIG);
    SOC = read_int_from_file(soc_path);
    char lite_path[MAX_PATH_LEN];
    snprintf(lite_path, sizeof(lite_path), "%s/lite_mode", MODULE_CONFIG);
    LITE_MODE = read_int_from_file(lite_path);
    char ppm_path[MAX_PATH_LEN];
    snprintf(ppm_path, sizeof(ppm_path), "%s/ppm_policies_mediatek", MODULE_CONFIG);
    read_string_from_file(PPM_POLICY, sizeof(PPM_POLICY), ppm_path);
    char custom_gov_path[MAX_PATH_LEN];
    snprintf(custom_gov_path, sizeof(custom_gov_path), "%s/custom_default_cpu_gov", MODULE_CONFIG);
    if (file_exists(custom_gov_path)) {
        read_string_from_file(DEFAULT_CPU_GOV, sizeof(DEFAULT_CPU_GOV), custom_gov_path);
    } else {
        char default_gov_path[MAX_PATH_LEN];
        snprintf(default_gov_path, sizeof(default_gov_path), "%s/default_cpu_gov", MODULE_CONFIG);
        read_string_from_file(DEFAULT_CPU_GOV, sizeof(DEFAULT_CPU_GOV), default_gov_path);
    }
    char mitigation_path[MAX_PATH_LEN];
    snprintf(mitigation_path, sizeof(mitigation_path), "%s/device_mitigation", MODULE_CONFIG);
    DEVICE_MITIGATION = read_int_from_file(mitigation_path);
}

void set_dnd(int mode) {
    if (mode == 0) {
        system("cmd notification set_dnd off");
    } else if (mode == 1) {
        system("cmd notification set_dnd priority");
    }
}

void perfcommon() {
    // Disable Kernel panic
    apply("0", "/proc/sys/kernel/panic");
    apply("0", "/proc/sys/kernel/panic_on_oops");
    apply("0", "/proc/sys/kernel/panic_on_warn");
    apply("0", "/proc/sys/kernel/softlockup_panic");
    
    // Sync to data
    sync();
    
    // I/O Tweaks
    DIR *dir = opendir("/sys/block");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] != '.') {
                char iostats_path[MAX_PATH_LEN];
                snprintf(iostats_path, sizeof(iostats_path), "/sys/block/%s/queue/iostats", ent->d_name);
                apply("0", iostats_path);
                
                char random_path[MAX_PATH_LEN];
                snprintf(random_path, sizeof(random_path), "/sys/block/%s/queue/add_random", ent->d_name);
                apply("0", random_path);
            }
        }
        closedir(dir);
    }
    
    // Networking tweaks
    const char *algorithms[] = {"bbr3", "bbr2", "bbrplus", "bbr", "westwood", "cubic"};
    for (int i = 0; i < 6; i++) {
        char cmd[MAX_PATH_LEN];
        snprintf(cmd, sizeof(cmd), "grep -q \"%s\" /proc/sys/net/ipv4/tcp_available_congestion_control", algorithms[i]);
        if (system(cmd) == 0) {
            apply(algorithms[i], "/proc/sys/net/ipv4/tcp_congestion_control");
            break;
        }
    }
        
    apply("1", "/proc/sys/net/ipv4/tcp_sack");
    apply("1", "/proc/sys/net/ipv4/tcp_ecn");
    apply("1", "/proc/sys/net/ipv4/tcp_window_scaling");
    apply("1", "/proc/sys/net/ipv4/tcp_moderate_rcvbuf");
    apply("3", "/proc/sys/net/ipv4/tcp_fastopen");
    
    // TCP buffer scaling
    apply("4096 87380 33554432", "/proc/sys/net/ipv4/tcp_rmem");
    apply("4096 65536 33554432", "/proc/sys/net/ipv4/tcp_wmem");

    // Global socket limit
    apply("33554432", "/proc/sys/net/core/rmem_max");
    apply("33554432", "/proc/sys/net/core/wmem_max");
    
    // Limit max perf event processing time
    apply("3", "/proc/sys/kernel/perf_cpu_time_max_percent");
    
    // VM Writeback Control
    apply("0", "/proc/sys/vm/page-cluster");
    apply("15", "/proc/sys/vm/stat_interval");
    apply("80", "/proc/sys/vm/overcommit_ratio");
    apply("640", "/proc/sys/vm/extfrag_threshold");
    apply("22",  "/proc/sys/vm/watermark_scale_factor");
    
    // Disable schedstats
    apply("0", "/proc/sys/kernel/sched_schedstats");
    
    // Disable Sched auto group
    apply("0", "/proc/sys/kernel/sched_autogroup_enabled");
    
    // Enable CRF
    apply("1", "/proc/sys/kernel/sched_child_runs_first");
    
    // Disable Oppo/Realme cpustats
    apply("0", "/proc/sys/kernel/task_cpustats_enable");
                
    // Disable SPI CRC
    apply("0", "/sys/module/mmc_core/parameters/use_spi_crc");
    
    // Disable OnePlus opchain
    apply("0", "/sys/module/opchain/parameters/chain_on");
    
    // Disable Oplus bloats
    apply("0", "/sys/module/cpufreq_bouncing/parameters/enable");
    apply("0", "/proc/task_info/task_sched_info/task_sched_info_enable");
    apply("0", "/proc/oplus_scheduler/sched_assist/sched_assist_enabled");
    
    // Reduce kernel log noise
    apply("0 0 0 0", "/proc/sys/kernel/printk");
    
    // Background Locality
    apply("0", "/dev/cpuset/background/memory_spread_page");
    apply("0", "/dev/cpuset/system-background/memory_spread_page");        
    
    // Report max CPU capabilities
    apply("libunity.so, libil2cpp.so, libmain.so, libUE4.so, libgodot_android.so, libgdx.so, libgdx-box2d.so, libminecraftpe.so, libLive2DCubismCore.so, libyuzu-android.so, libryujinx.so, libcitra-android.so, libhdr_pro_engine.so, libandroidx.graphics.path.so, libeffect.so", "/proc/sys/kernel/sched_lib_name");
    apply("255", "/proc/sys/kernel/sched_lib_mask_force");
        
    // Set thermal governor to step_wise
    DIR *thermal_dir = opendir("/sys/class/thermal");
    if (thermal_dir) {
        struct dirent *ent;
        while ((ent = readdir(thermal_dir)) != NULL) {
            if (strstr(ent->d_name, "thermal_zone")) {
                char policy_path[MAX_PATH_LEN];
                snprintf(policy_path, sizeof(policy_path), "/sys/class/thermal/%s/policy", ent->d_name);
                apply("step_wise", policy_path);
            }
        }
        closedir(thermal_dir);
    }
}

void esport_mode() {
    // Enable Do not Disturb
    char dnd_path[MAX_PATH_LEN];
    snprintf(dnd_path, sizeof(dnd_path), "%s/dnd_gameplay", MODULE_CONFIG);
    if (read_int_from_file(dnd_path) == 1) {
        set_dnd(1);
    }
                
    // Disable battery saver
    apply("N", "/sys/module/workqueue/parameters/power_efficient");
    
    // Increase network device backlog
    apply("3500", "/proc/sys/net/core/netdev_max_backlog");

    // Adjust TCP auto-scaling buffer
    apply("4096 131072 33554432", "/proc/sys/net/ipv4/tcp_rmem");
    apply("4096 131072 33554432", "/proc/sys/net/ipv4/tcp_wmem");
    
    // Disable split lock mitigation
    apply("0", "/proc/sys/kernel/split_lock_mitigate");
    
    // Memory tweak
    apply("20", "/proc/sys/vm/swappiness");
    apply("60", "/proc/sys/vm/vfs_cache_pressure");
    apply("30", "/proc/sys/vm/compaction_proactiveness");
    
    // VM Writeback Control
    apply("5", "/proc/sys/vm/dirty_background_ratio");
    apply("15", "/proc/sys/vm/dirty_ratio");
    apply("1500", "/proc/sys/vm/dirty_expire_centisecs");
    apply("1500", "/proc/sys/vm/dirty_writeback_centisecs");
    
    // Foreground Priority Spread
    apply("1", "/dev/cpuset/top-app/memory_spread_page");
    apply("1", "/dev/cpuset/foreground/memory_spread_page");
    
    // Energy Aware Scheduling Policy
    apply("0", "/proc/sys/kernel/sched_energy_aware");
    
    // CPU Priority Control
    apply("768", "/dev/cpuctl/top-app/cpu.shares");
    apply("512", "/dev/cpuctl/foreground/cpu.shares");

    // Background Limits
    apply("64", "/dev/cpuctl/background/cpu.shares");
    apply("96", "/dev/cpuctl/system-background/cpu.shares");

    // Services Priority
    apply("192", "/dev/cpuctl/nnapi-hal/cpu.shares");
    apply("192", "/dev/cpuctl/dex2oat/cpu.shares");

    // System Core
    apply("512", "/dev/cpuctl/system/cpu.shares");
    
    // Strongly limit background burst
    apply("128", "/dev/cpuctl/background/cpu.uclamp.max");
    apply("128", "/dev/cpuctl/system-background/cpu.uclamp.max");
    apply("128", "/dev/cpuctl/restricted-background/cpu.uclamp.max");

    // Guarantee top-app minimum
    apply("256", "/sys/fs/cgroup/uclamp/top-app.uclamp.min");
    apply("128", "/sys/fs/cgroup/uclamp/foreground.uclamp.min");

    // Keep background minimal baseline
    apply("32", "/sys/fs/cgroup/uclamp/background.uclamp.min");
    
    // Sched Boost
    apply("1", "/proc/sys/kernel/sched_boost");
    
    // Sched Boost Old
    apply("1", "/dev/stune/top-app/schedtune.prefer_idle");
    apply("1", "/dev/stune/top-app/schedtune.boost");          
    
    // Improve real time latencies
    apply("32", "/proc/sys/kernel/sched_nr_migrate");
    
    // Scheduler Timeslice
    apply("4", "/proc/sys/kernel/sched_rr_timeslice_ms");

    // Load Tracking Speed
    apply("80", "/proc/sys/kernel/sched_time_avg_ms");

    // Scheduler Scaling Logic
    apply("1", "/proc/sys/kernel/sched_tunable_scaling");
    
    // Tweaking scheduler
    apply("15", "/proc/sys/kernel/sched_min_task_util_for_boost");
    apply("8", "/proc/sys/kernel/sched_min_task_util_for_colocation");
    apply("50000",  "/proc/sys/kernel/sched_migration_cost_ns");
    apply("800000", "/proc/sys/kernel/sched_min_granularity_ns");
    apply("900000", "/proc/sys/kernel/sched_wakeup_granularity_ns");
    
    // Sched Features
    apply("NEXT_BUDDY", "/sys/kernel/debug/sched_features");
    apply("NO_TTWU_QUEUE", "/sys/kernel/debug/sched_features");
            
    // Oppo/Oplus/Realme Touchpanel
    apply("1", "/proc/touchpanel/game_switch_enable");
    apply("0", "/proc/touchpanel/oplus_tp_limit_enable");
    apply("0", "/proc/touchpanel/oppo_tp_limit_enable");
    apply("1", "/proc/touchpanel/oplus_tp_direction");
    apply("1", "/proc/touchpanel/oppo_tp_direction");
    
    // eMMC and UFS frequency
    DIR *dir = opendir("/sys/class/devfreq");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            char *name = ent->d_name;
            if (strstr(name, ".ufshc") || strstr(name, "mmc")) {
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
    
    // Set CPU governor
    if (LITE_MODE == 0 && DEVICE_MITIGATION == 0) {
        change_cpu_gov("performance");
    } else {
        change_cpu_gov(DEFAULT_CPU_GOV);
    }
    
    // Force CPU to highest possible frequency
    if (file_exists("/proc/ppm")) {
        cpufreq_ppm_max_perf();
    } else {
        cpufreq_max_perf();
    }
    
    // I/O Tweaks
    const char *block_devs[] = {"mmcblk0", "mmcblk1"};
    for (int i = 0; i < 2; i++) {
        char read_ahead_path[MAX_PATH_LEN];
        snprintf(read_ahead_path, sizeof(read_ahead_path), "/sys/block/%s/queue/read_ahead_kb", block_devs[i]);
        if (file_exists(read_ahead_path)) {
            apply("32", read_ahead_path);
        }
        
        char nr_requests_path[MAX_PATH_LEN];
        snprintf(nr_requests_path, sizeof(nr_requests_path), "/sys/block/%s/queue/nr_requests", block_devs[i]);
        if (file_exists(nr_requests_path)) {
            apply("32", nr_requests_path);
        }
    }
    
    // Process SD cards
    dir = opendir("/sys/block");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, "sd")) {
                char read_ahead_path[MAX_PATH_LEN];
                snprintf(read_ahead_path, sizeof(read_ahead_path), "/sys/block/%s/queue/read_ahead_kb", ent->d_name);
                if (file_exists(read_ahead_path)) {
                    apply("32", read_ahead_path);
                }
                
                char nr_requests_path[MAX_PATH_LEN];
                snprintf(nr_requests_path, sizeof(nr_requests_path), "/sys/block/%s/queue/nr_requests", ent->d_name);
                if (file_exists(nr_requests_path)) {
                    apply("32", nr_requests_path);
                }
            }
        }
        closedir(dir);
    }
            
    // SOC-specific esport tweaks
    switch (SOC) {
        case 1: mediatek_esport(); break;
        case 2: snapdragon_esport(); break;
        case 3: exynos_esport(); break;
        case 4: unisoc_esport(); break;
        case 5: tensor_esport(); break;
    }
}

void balanced_mode() {
    char dnd_path[MAX_PATH_LEN];
    snprintf(dnd_path, sizeof(dnd_path), "%s/dnd_gameplay", MODULE_CONFIG);
    if (read_int_from_file(dnd_path) == 1) {
        set_dnd(0);
    }
    
    // Disable battery saver
    apply("N", "/sys/module/workqueue/parameters/power_efficient");
    
    // Increase network device backlog
    apply("3000", "/proc/sys/net/core/netdev_max_backlog");

    // Adjust TCP auto-scaling buffer
    apply("4096 87380 33554432", "/proc/sys/net/ipv4/tcp_rmem");
    apply("4096 65536 33554432", "/proc/sys/net/ipv4/tcp_wmem");
    
    // Enable split lock mitigation
    apply("1", "/proc/sys/kernel/split_lock_mitigate");
    
    // Memory Tweaks
    apply("40", "/proc/sys/vm/swappiness");
    apply("80", "/proc/sys/vm/vfs_cache_pressure");
    apply("40", "/proc/sys/vm/compaction_proactiveness");
        
    // VM Writeback Control
    apply("10", "/proc/sys/vm/dirty_background_ratio");
    apply("25", "/proc/sys/vm/dirty_ratio");
    apply("3000", "/proc/sys/vm/dirty_expire_centisecs");
    apply("3000", "/proc/sys/vm/dirty_writeback_centisecs");
    
    // Foreground Priority Spread
    apply("1", "/dev/cpuset/top-app/memory_spread_page");
    apply("0", "/dev/cpuset/foreground/memory_spread_page");
    
    // Energy Aware Scheduling Policy
    apply("1", "/proc/sys/kernel/sched_energy_aware");

    // CPU Priority Control
    apply("512", "/dev/cpuctl/top-app/cpu.shares");
    apply("384", "/dev/cpuctl/foreground/cpu.shares");

    // Background Limits
    apply("128", "/dev/cpuctl/background/cpu.shares");
    apply("192", "/dev/cpuctl/system-background/cpu.shares");

    // Services Priority
    apply("192", "/dev/cpuctl/nnapi-hal/cpu.shares");
    apply("192", "/dev/cpuctl/dex2oat/cpu.shares");

    // System Core
    apply("512", "/dev/cpuctl/system/cpu.shares");

    // Moderate background restriction
    apply("256", "/dev/cpuctl/background/cpu.uclamp.max");
    apply("256", "/dev/cpuctl/system-background/cpu.uclamp.max");
    apply("256", "/dev/cpuctl/restricted-background/cpu.uclamp.max");

    // Balanced minimum floor
    apply("192", "/sys/fs/cgroup/uclamp/top-app.uclamp.min");
    apply("96",  "/sys/fs/cgroup/uclamp/foreground.uclamp.min");

    // Keep background baseline
    apply("32",  "/sys/fs/cgroup/uclamp/background.uclamp.min");

    // Sched Boost
    apply("0", "/proc/sys/kernel/sched_boost");
    
    // Sched Boost old
    apply("0", "/dev/stune/top-app/schedtune.prefer_idle");
    apply("1", "/dev/stune/top-app/schedtune.boost");
    
    // Scheduler Timeslice
    apply("6", "/proc/sys/kernel/sched_rr_timeslice_ms");

    // Load Tracking Speed
    apply("100", "/proc/sys/kernel/sched_time_avg_ms");

    // Scheduler Scaling Logic
    apply("1", "/proc/sys/kernel/sched_tunable_scaling");
    
    // Improve real time latencies
    apply("16", "/proc/sys/kernel/sched_nr_migrate");
    
    // Tweaking scheduler
    apply("25", "/proc/sys/kernel/sched_min_task_util_for_boost");
    apply("15", "/proc/sys/kernel/sched_min_task_util_for_colocation");
    apply("100000", "/proc/sys/kernel/sched_migration_cost_ns");
    apply("1200000", "/proc/sys/kernel/sched_min_granularity_ns");
    apply("2000000", "/proc/sys/kernel/sched_wakeup_granularity_ns");
    
    // Sched Features
    apply("NEXT_BUDDY", "/sys/kernel/debug/sched_features");
    apply("TTWU_QUEUE", "/sys/kernel/debug/sched_features");
        
    // Oppo/Oplus/Realme Touchpanel
    apply("0", "/proc/touchpanel/game_switch_enable");
    apply("1", "/proc/touchpanel/oplus_tp_limit_enable");
    apply("1", "/proc/touchpanel/oppo_tp_limit_enable");
    apply("0", "/proc/touchpanel/oplus_tp_direction");
    apply("0", "/proc/touchpanel/oppo_tp_direction");
    
    // eMMC and UFS frequency
    DIR *dir = opendir("/sys/class/devfreq");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            char *name = ent->d_name;
            if (strstr(name, ".ufshc") || strstr(name, "mmc")) {
                char path[MAX_PATH_LEN];
                snprintf(path, sizeof(path), "/sys/class/devfreq/%s", name);
                devfreq_unlock(path);
            }
        }
        closedir(dir);
    }
    
    // Restore CPU settings
    change_cpu_gov(DEFAULT_CPU_GOV);
    if (file_exists("/proc/ppm")) {
        cpufreq_ppm_unlock();
    } else {
        cpufreq_unlock();
    }
    
    // I/O Tweaks
    const char *block_devs[] = {"mmcblk0", "mmcblk1"};
    for (int i = 0; i < 2; i++) {
        char read_ahead_path[MAX_PATH_LEN];
        snprintf(read_ahead_path, sizeof(read_ahead_path), "/sys/block/%s/queue/read_ahead_kb", block_devs[i]);
        if (file_exists(read_ahead_path)) {
            apply("64", read_ahead_path);
        }
        
        char nr_requests_path[MAX_PATH_LEN];
        snprintf(nr_requests_path, sizeof(nr_requests_path), "/sys/block/%s/queue/nr_requests", block_devs[i]);
        if (file_exists(nr_requests_path)) {
            apply("64", nr_requests_path);
        }
    }
    
    // Process SD cards
    dir = opendir("/sys/block");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, "sd")) {
                char read_ahead_path[MAX_PATH_LEN];
                snprintf(read_ahead_path, sizeof(read_ahead_path), "/sys/block/%s/queue/read_ahead_kb", ent->d_name);
                if (file_exists(read_ahead_path)) {
                    apply("128", read_ahead_path);
                }
                
                char nr_requests_path[MAX_PATH_LEN];
                snprintf(nr_requests_path, sizeof(nr_requests_path), "/sys/block/%s/queue/nr_requests", ent->d_name);
                if (file_exists(nr_requests_path)) {
                    apply("64", nr_requests_path);
                }
            }
        }
        closedir(dir);
    }
           
    // SOC-specific balanced tweaks
    switch (SOC) {
        case 1: mediatek_balanced(); break;
        case 2: snapdragon_balanced(); break;
        case 3: exynos_balanced(); break;
        case 4: unisoc_balanced(); break;
        case 5: tensor_balanced(); break;
    }
}

void efficiency_mode() {
    char dnd_path[MAX_PATH_LEN];
    snprintf(dnd_path, sizeof(dnd_path), "%s/dnd_gameplay", MODULE_CONFIG);
    if (read_int_from_file(dnd_path) == 1) {
        set_dnd(0);
    }    

    // Enable battery saver module
    apply("Y", "/sys/module/workqueue/parameters/power_efficient");
    
    // Increase network device backlog
    apply("2500", "/proc/sys/net/core/netdev_max_backlog");

    // Adjust TCP auto-scaling buffer
    apply("4096 65536 16777216", "/proc/sys/net/ipv4/tcp_rmem");
    apply("4096 65536 16777216", "/proc/sys/net/ipv4/tcp_wmem");
    
    // Enable split lock mitigation
    apply("1", "/proc/sys/kernel/split_lock_mitigate");
    
    // Memory Tweaks
    apply("60", "/proc/sys/vm/swappiness");
    apply("100", "/proc/sys/vm/vfs_cache_pressure");
    apply("10", "/proc/sys/vm/compaction_proactiveness");
    
    // VM Writeback Control
    apply("20", "/proc/sys/vm/dirty_background_ratio");
    apply("40", "/proc/sys/vm/dirty_ratio");
    apply("6000", "/proc/sys/vm/dirty_writeback_centisecs");
    apply("6000", "/proc/sys/vm/dirty_expire_centisecs");
    
    // Foreground Priority Spread
    apply("0", "/dev/cpuset/top-app/memory_spread_page");
    apply("0", "/dev/cpuset/foreground/memory_spread_page");
    
    // Energy Aware Scheduling Policy
    apply("1", "/proc/sys/kernel/sched_energy_aware");

    // CPU Priority Control
    apply("384", "/dev/cpuctl/top-app/cpu.shares");
    apply("256", "/dev/cpuctl/foreground/cpu.shares");

    // Background Limits
    apply("128", "/dev/cpuctl/background/cpu.shares");
    apply("128", "/dev/cpuctl/system-background/cpu.shares");

    // Services Priority
    apply("128", "/dev/cpuctl/nnapi-hal/cpu.shares");
    apply("128", "/dev/cpuctl/dex2oat/cpu.shares");

    // System Core
    apply("384", "/dev/cpuctl/system/cpu.shares");
    
    // Keep background controlled
    apply("128", "/dev/cpuctl/background/cpu.uclamp.max");
    apply("128", "/dev/cpuctl/system-background/cpu.uclamp.max");
    apply("128", "/dev/cpuctl/restricted-background/cpu.uclamp.max");

    // Lower performance floor
    apply("128", "/sys/fs/cgroup/uclamp/top-app.uclamp.min");
    apply("64",  "/sys/fs/cgroup/uclamp/foreground.uclamp.min");

    // Keep background baseline
    apply("16",  "/sys/fs/cgroup/uclamp/background.uclamp.min"); 
    
    // Sched Boost
    apply("0", "/proc/sys/kernel/sched_boost");
    
    // Sched Boost Old
    apply("1", "/dev/stune/top-app/schedtune.prefer_idle");
    apply("0", "/dev/stune/top-app/schedtune.boost");
    
    // Scheduler Timeslice
    apply("10", "/proc/sys/kernel/sched_rr_timeslice_ms");

    // Load Tracking Speed
    apply("150", "/proc/sys/kernel/sched_time_avg_ms");

    // Scheduler Scaling Logic
    apply("1", "/proc/sys/kernel/sched_tunable_scaling");
    
    // Improve real time latencies
    apply("8", "/proc/sys/kernel/sched_nr_migrate");
    
    // Tweaking scheduler
    apply("45", "/proc/sys/kernel/sched_min_task_util_for_boost");
    apply("30", "/proc/sys/kernel/sched_min_task_util_for_colocation");
    apply("200000", "/proc/sys/kernel/sched_migration_cost_ns");
    apply("2000000", "/proc/sys/kernel/sched_min_granularity_ns");
    apply("3000000", "/proc/sys/kernel/sched_wakeup_granularity_ns");
    
    // Sched Features
    apply("NO_NEXT_BUDDY", "/sys/kernel/debug/sched_features");
    apply("TTWU_QUEUE", "/sys/kernel/debug/sched_features");
        
    // Oppo/Oplus/Realme Touchpanel
    apply("0", "/proc/touchpanel/game_switch_enable");
    apply("1", "/proc/touchpanel/oplus_tp_limit_enable");
    apply("1", "/proc/touchpanel/oppo_tp_limit_enable");
    apply("0", "/proc/touchpanel/oplus_tp_direction");
    apply("0", "/proc/touchpanel/oppo_tp_direction");
        
    // eMMC and UFS frequency
    DIR *dir = opendir("/sys/class/devfreq");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            char *name = ent->d_name;
            if (strstr(name, ".ufshc") || strstr(name, "mmc")) {
                char path[MAX_PATH_LEN];
                snprintf(path, sizeof(path), "/sys/class/devfreq/%s", name);
                devfreq_min_perf(path);
            }
        }
        closedir(dir);
    }
    
    // CPU governor for efficiency
    char efficiency_gov_path[MAX_PATH_LEN];
    snprintf(efficiency_gov_path, sizeof(efficiency_gov_path), "%s/efficiency_cpu_gov", MODULE_CONFIG);
    char efficiency_gov[50];
    read_string_from_file(efficiency_gov, sizeof(efficiency_gov), efficiency_gov_path);
    change_cpu_gov(efficiency_gov);
    
    // I/O Tweaks
    const char *block_devs[] = {"mmcblk0", "mmcblk1"};
    for (int i = 0; i < 2; i++) {
        char read_ahead_path[MAX_PATH_LEN];
        snprintf(read_ahead_path, sizeof(read_ahead_path), "/sys/block/%s/queue/read_ahead_kb", block_devs[i]);
        if (file_exists(read_ahead_path)) {
            apply("16", read_ahead_path);
        }
        
        char nr_requests_path[MAX_PATH_LEN];
        snprintf(nr_requests_path, sizeof(nr_requests_path), "/sys/block/%s/queue/nr_requests", block_devs[i]);
        if (file_exists(nr_requests_path)) {
            apply("16", nr_requests_path);
        }
    }

    // Process SD cards
    dir = opendir("/sys/block");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, "sd")) {
                char read_ahead_path[MAX_PATH_LEN];
                snprintf(read_ahead_path, sizeof(read_ahead_path), "/sys/block/%s/queue/read_ahead_kb", ent->d_name);
                if (file_exists(read_ahead_path)) {
                    apply("128", read_ahead_path);
                }
                
                char nr_requests_path[MAX_PATH_LEN];
                snprintf(nr_requests_path, sizeof(nr_requests_path), "/sys/block/%s/queue/nr_requests", ent->d_name);
                if (file_exists(nr_requests_path)) {
                    apply("64", nr_requests_path);
                }
            }
        }
        closedir(dir);
    }
                
    // SOC-specific efficiency tweaks
    switch (SOC) {
        case 1: mediatek_efficiency(); break;
        case 2: snapdragon_efficiency(); break;
        case 3: exynos_efficiency(); break;
        case 4: unisoc_efficiency(); break;
        case 5: tensor_efficiency(); break;
    }
}
