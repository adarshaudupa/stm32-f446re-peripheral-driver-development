/*
 * clock.c
 *
 *  Created on: Mar 9, 2026
 *      Author: Adarsha Udupa
 */

#include "stm32f4xx.h"
#include "clock.h"

uint32_t get_sysclk_freq_hz(void) {
    // Read SWS bits [3:2]
    uint32_t sws = (RCC->CFGR >> 2) & 0x3;

    switch (sws) {
    case 0x0: // HSI
        return 16000000;
    case 0x1: // HSE (assume 8 MHz if ever used)
        return 8000000;
    case 0x2: // PLL
        // For now, assume you're NOT using PLL. If SWS == 2 here,
        // your CubeMX config is different and you must handle PLL properly.
        return 16000000;
    default:
        return 16000000;
    }
}

static uint32_t get_ahb_freq_hz(void) {
    uint32_t sysclk = get_sysclk_freq_hz();
    uint32_t hpre   = (RCC->CFGR >> 4) & 0xF;

    if (hpre < 8) {
        return sysclk;          // 0xxx: no division
    } else {
        uint32_t shift = hpre - 7; // 1000→1, 1001→2, ...
        return sysclk >> shift;
    }
}

uint32_t get_apb1_freq_hz(void) {
    uint32_t hclk  = get_ahb_freq_hz();
    uint32_t ppre1 = (RCC->CFGR >> 10) & 0x7;

    if (ppre1 < 4) {
        return hclk;            // 0xx: no division
    } else {
        uint32_t shift = ppre1 - 3; // 100→1, 101→2, ...
        return hclk >> shift;
    }
}

uint32_t get_apb2_freq_hz(void) {
    uint32_t hclk = get_ahb_freq_hz();
    uint32_t ppre2 = (RCC->CFGR >> 13) & 0x7;

    if (ppre2 < 4) {
        return hclk;
    } else {
        uint32_t shift = ppre2 - 3;
        return hclk >> shift;
    }
}

