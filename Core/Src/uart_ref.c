#include "uart.h"
#include "stm32f4xx.h"

#define BUFFER 64

volatile char buffer[BUFFER];
volatile uint8_t head=0;
volatile uint8_t tail=0;

void UART2_init(void)
{
 RCC->AHB1ENR |= (1<<0);
 RCC->APB1ENR |= (1<<17);
 
 GPIOA->MODER &= ~((3<<4) | (3<<6));
 GPIOA->MODER |= (2<<4) | (2<<6);
 
 GPIOA->AFR[0] &= ~((0xF<<8) | (0xF<<12));
 GPIOA->AFR[0] |= (7<<8) | (7<<12);
 
 USART2->BRR = 0x683;
 
 USART2->CR1 |= (1<<13) | (1<<2) | (1<<3) | (1<<5);
 
 NVIC->ISER[1] |= (1<<6);
}

void UART_TChar(char ch)
{
 while(!(USART2->SR&(1<<7)));
 USART2->DR=ch;
}

char USART2_RChar(void)
{
 while(head==tail);
 char rc=buffer[tail];
 tail++;
 if(tail>=BUFFER)
   tail=0;
 return rc;
}


