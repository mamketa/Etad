#ifndef VELFOX_SOC_H
#define VELFOX_SOC_H

#include "velfox_common.h"

// Mediatek functions
void mediatek_apex();
void mediatek_adaptive();
void mediatek_efficiency();

// Snapdragon functions
void snapdragon_apex();
void snapdragon_adaptive();
void snapdragon_efficiency();

// Exynos functions
void exynos_apex();
void exynos_adaptive();
void exynos_efficiency();

// Unisoc functions
void unisoc_apex();
void unisoc_adaptive();
void unisoc_efficiency();

// Tensor functions
void tensor_apex();
void tensor_adaptive();
void tensor_efficiency();

#endif // VELFOX_SOC_H