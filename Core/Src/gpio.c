/*
 * gpio.c
 *
 *  Created on: Feb 25, 2026
 *      Author: Adarsha Udupa
 */

#include "stm32f4xx.h"
#include "gpio.h"

void PA5_Init(void)
{
 RCC->AHB1ENR |= (1<<0); //Enable GPIOA Clock
 GPIOA->MODER &= ~(3<<10);
 GPIOA->MODER |= (1<<10); //Set PA5 to Output Mode
}

void LED_ON(void)
{
 GPIOA->ODR |= (1<<5);
}

void LED_OFF(void)
{
 GPIOA->ODR &= ~(1<<5);
}

void LED_Toggle(void)
{
 GPIOA->ODR ^= (1<<5);
}

void PC13_Init(void)
{
 RCC->AHB1ENR |= (1<<2); //Enable GPIOC Clock
 GPIOC->MODER &= ~(3<<26); //Sets PC13 to Input Mode
}

uint8_t Button_Read(void)
{
 if(GPIOC->IDR & (1 << 13)) return 0;
 else return 1;
}
