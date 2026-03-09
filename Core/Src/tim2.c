/*
 * tim2.c
 *
 *  Created on: Feb 13, 2026
 *      Author: Adarsha Udupa
 */

#include "stm32f446xx.h"
#include "tim2.h"
#include "clock.h"
#include "gpio.h"

volatile uint8_t tim2_flag = 0;


void timer_start(void) {
    TIM2->CR1 |= (1 << 0);
}

void timer_stop(void) {
    TIM2->CR1 &= ~(1 << 0);
    TIM2->CNT = 0;
}


void TIM2_Init(void) {
    // Enable TIM2 clock (bit 0 of APB1ENR)
    RCC->APB1ENR |= (1 << 0);

    uint32_t tim_clk = get_apb1_freq_hz(); // currently 16 MHz
    uint32_t target_hz = 1;
    uint32_t tick_hz   = 10000;

    // Choose a convenient tick frequency, say 10 kHz:
    // tim_clk / ((PSC+1)*(ARR+1)) = target_hz
    // Set PSC so (PSC+1) = tim_clk / 10000 → ARR = 10000/target_hz - 1
    uint32_t psc = (tim_clk / tick_hz) - 1;             // ~1599
    uint32_t arr = (tick_hz / target_hz) - 1;           // 9999

    // Set prescaler
    TIM2->PSC = psc;

    // Set auto-reload
    TIM2->ARR = arr;

    // Enable update interrupt (bit 0 of DIER)
    TIM2->DIER |= (1 << 0);

    // Enable TIM2 interrupt in NVIC
    NVIC_EnableIRQ(TIM2_IRQn);  // This one stays as-is (CMSIS function)
    NVIC_SetPriority(TIM2_IRQn, 1);  // Lower priority number = higher priority
}

void TIM2_IRQHandler(void)
{
    if (TIM2->SR & (1<<0)) {
        TIM2->SR &= ~(1<<0);
        tim2_flag = 1;
        LED_Toggle();
    }
}






