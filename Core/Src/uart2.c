/*
 * uart.c
 *
 * UART driver implementation for STM32F446RE
 */

#include "uart2.h"
#include "stm32f4xx.h"

// Circular Buffer for RX

#define RX_BUFFER_SIZE 64

volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_head = 0;
volatile uint8_t rx_tail = 0;

void UART2_Init() {

	RCC->AHB1ENR |= (1<<0);
	RCC->APB1ENR |= (1<<17); //Enable GPIOA Clock

	GPIOA->MODER &= ~((3<<4) | (3<<6));
	GPIOA->MODER |= (2<<4) | (2<<6); //Set PA2(TX) and PA3(RX) in AF Mode

	GPIOA->AFR[0] &= ~((0xF<<8) | (0xF<<12));
	GPIOA->AFR[0] |= (7<<8) | (7<<12); //Set PA2 and PA3 to AF7(UART2) in AFRL register
	// Formula: BRR = f_PCLK / (16 * baudrate)
	// APB1 clock = 16 MHz (default)
	// For 9600: BRR = 16000000 / (16 * 9600) = 104.166
	USART2->BRR = 0x683;

	USART2->CR1 = (1 << 13) | (1 << 3) | (1 << 2); //Enable UE(USART Enable), TE and RE
	USART2->CR1 |= (1 << 5); //Enable RXNE interrupt

	NVIC->ISER[1] |= (1 << 6);  // Enable USART2 interrupt in NVIC

	    // Flush any garbage data
	for (volatile int i = 0; i < 1000; i++);
	    if (USART2->SR & (1 << 5)) {
	        volatile uint32_t temp = USART2->DR;
	        (void)temp;
}
}

void UART2_SendChar(char ch) {
    // SR bit 7: TXE = 1 when DR is empty (safe to write)
    // Blocking wait: spin here until TXE = 1
    while (!(USART2->SR & (1 << 7)));
    USART2->DR = ch;
}

void UART2_SendString(char *str) {
    while (*str) {
        UART2_SendChar(*str);  // Send current character
        str++;                 // Move to next character
    }
}
//char msg[] = "Hello";
// Memory: ['H']['e']['l']['l']['o']['\0']
//           ↑
//          str points here initially
// *str = 'H' → send it
// str++ → now points to 'e'
// Loop until *str = '\0' (null terminator)

char UART2_ReadChar(void) {
    // Wait for data (buffer not empty)
    while (rx_head == rx_tail);

    // Read from buffer
    char c = rx_buffer[rx_tail];

    // Move tail forward
    rx_tail++;

    // Wrap around if needed
    if (rx_tail >= RX_BUFFER_SIZE) {
        rx_tail = 0;
    }

    return c;
}



// DR is 9 bits: [8][7][6][5][4][3][2][1][0]
//                ↑
//           Bit 8 = parity (we don't use it)

// & 0xFF masks to only bits [7:0]
/*
 * 0xFF = 0b11111111 (8 bits)

Example:
DR = 0b100101010 (9 bits, 'A' with parity bit set)
DR & 0xFF = 0b001000001 = 0x41 = 'A' ✓
*/


uint8_t UART2_DataAvailable(void) {
    return (rx_head != rx_tail);
}

void USART2_IRQHandler(void) {
    // Check if RXNE (byte received)
    if (USART2->SR & (1 << 5)) {
        // Read byte (clears RXNE flag)
        char received_byte = USART2->DR;

        // Store in buffer
        rx_buffer[rx_head] = received_byte;

        // Move head forward
        rx_head++;

        // Wrap around if needed
        if (rx_head >= RX_BUFFER_SIZE) {
            rx_head = 0;
        }
    }
}
