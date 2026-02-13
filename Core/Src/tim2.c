/*
 * tim2.c
 *
 *  Created on: Feb 13, 2026
 *      Author: Adarsha Udupa
 */

#include "stm32f446xx.h"
#include "tim2.h"

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

    // Set prescaler
    TIM2->PSC = 8399;

    // Set auto-reload
    TIM2->ARR = 9999;

    // Enable update interrupt (bit 0 of DIER)
    TIM2->DIER |= (1 << 0);

    // Enable TIM2 interrupt in NVIC
    NVIC_EnableIRQ(TIM2_IRQn);  // This one stays as-is (CMSIS function)
    NVIC_SetPriority(TIM2_IRQn, 1);  // Lower priority number = higher priority
}


void TIM2_IRQHandler(void) {
    if (TIM2->SR & (1 << 0)) {  // Check UIF flag
        TIM2->SR &= ~(1 << 0);   // Clear UIF flag

        if (led_state == LED_AUTO_BLINK) {
            GPIOA->ODR ^= (1 << 5);
        }
    }
}




