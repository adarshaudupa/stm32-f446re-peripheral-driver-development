/*
 * hc-sr04.c
 *
 *  Created on: 17-Mar-2026
 *      Author: Adarsha Udupa
 */

#include "stm32f4xx.h"
#include "hc-sr04.h"
#include "tim2.h"

void HCSR04_Init(void)
{
	RCC->AHB1ENR |= (1<<0); //Enable GPIOA Clock
	GPIOA->MODER &= ~((3<<12) | (3<<14)); //Clear mode bits for PA6 and PA7
	GPIOA->MODER |= (1<<12);; //Set PA6 to output mode(PA7 is input)
	GPIOA->OTYPER &= ~(1<<6); //Set to push-pull output type
	GPIOA->OSPEEDR &= ~(3<<12);
	GPIOA->OSPEEDR |= (3<<12); //Set to high output speed
	GPIOA->PUPDR &= ~((3<<12) | (3<<14)); //Clear pull-up/pull-down bits for PA6 and PA7
	GPIOA->PUPDR |=  (2 << 14); //Set PA7 to pull-down input for stable LOW when idle

	TIM2_Init_1MHz(); // Initialize TIM2 to count microseconds
}

uint32_t HCSR04_ReadPulseUs(void)
{
    // Wait for rising edge (0 → 1)
    while (HCSR04_Echo_Read() == 0) {
        // spin
    }
    uint32_t start = TIM2->CNT;

    // Wait for falling edge (1 → 0)
    while (HCSR04_Echo_Read() == 1) {
        // spin
    }
    uint32_t end = TIM2->CNT;

    return end - start;    // microseconds
}


static inline uint32_t HCSR04_Echo_Read(void)
{
    return (GPIOA->IDR >> 7) & 0x1;        // IDR bit7
}


void HCSR04_Trigger(void)
{
    // Ensure low
    GPIOA->BSRR = (1 << (6 + 16));   // reset PA6(low)

    // Short delay ~10 us (using the 1 MHz timer)
    uint32_t start = TIM2->CNT;
    while ((TIM2->CNT - start) < 10) {
        // spin 10 us
    }

    // Drive high
    GPIOA->BSRR = (1 << 6);

    // Keep high for 10 us
    start = TIM2->CNT;
    while ((TIM2->CNT - start) < 10) {
        // spin 10 us
    }

    // Back low
    GPIOA->BSRR = (1 << (6 + 16));
}


uint32_t HCSR04_MeasureUs(void)
{
    HCSR04_Trigger();            // send 10 us pulse on TRIG
    return HCSR04_ReadPulseUs(); // wait echo and measure width
}
