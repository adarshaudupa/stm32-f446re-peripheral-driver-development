#include "adc1.h"
#include<stdlib.h>
#include "stm32f4xx.h"
void ADC1_Init(void)
{
    RCC->AHB1ENR |= (1<<0);  // Enable GPIOA clock
    RCC->APB2ENR |= (1<<8);  // Enable ADC1 clock

    GPIOA->MODER &= ~(3<<0);
    GPIOA->MODER |= (3<<0);  // PA0 analog mode

    GPIOA->MODER &= ~(3<<10);
    GPIOA->MODER |= (1<<10);  // PA5 output (LED)

    ADC1->CR2 |= (1<<0);  // ADON - power on ADC
    for(volatile int i=0; i<1000; i++);  // Stabilization delay

    ADC1->SQR3 = 0;  // Channel 0
}

uint8_t ADC1_Read()
{
 return ADC1->DR;
}

