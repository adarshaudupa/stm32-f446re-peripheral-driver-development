#include "stm32f4xx.h"

void UART2_Init(void) {
    // Step 1: Enable peripheral clocks
    RCC->AHB1ENR |= (1 << 0);      // Enable GPIOA clock
    RCC->APB1ENR |= (1 << 17);     // Enable USART2 clock

    // Step 2: Configure PA2 as alternate function for USART2_TX
    GPIOA->MODER &= ~(3 << 4);     // Clear bits [5:4] for PA2
    GPIOA->MODER |= (2 << 4);      // Set PA2 to alternate function mode (0b10)

    // Step 3: Select AF7 (USART2) for PA2
    GPIOA->AFR[0] &= ~(0xF << 8);  // Clear bits [11:8] for PA2
    GPIOA->AFR[0] |= (7 << 8);     // Set PA2 to AF7 (USART2_TX)

    // Step 4: Configure baud rate to 9600
    // BRR = 16MHz / (16 * 9600) = 104.166
    // Mantissa = 104 (0x68), Fraction = 3
    USART2->BRR = (104 << 4) | 3;  // Set baud rate register

    // Step 5: Enable USART and transmitter
    USART2->CR1 = (1 << 13) | (1 << 3);  // UE=1 (enable USART), TE=1 (enable TX)
}

void LED_Init(void) {
    // Enable GPIOA clock (redundant but safe)
    RCC->AHB1ENR |= (1 << 0);      // Enable GPIOA clock

    // Configure PA5 as output for LED
    GPIOA->MODER &= ~(3 << 10);    // Clear bits [11:10] for PA5
    GPIOA->MODER |= (1 << 10);     // Set PA5 to output mode (0b01)
}

void LED_Toggle(void) {
    // Toggle PA5 using XOR operation
    GPIOA->ODR ^= (1 << 5);        // Flip bit 5: 0→1 or 1→0
}

void UART2_SendChar(char c) {
    // Wait until transmit data register is empty
    while (!(USART2->SR & (1 << 7)));  // Loop while TXE flag (bit 7) is 0

    // Write character to data register
    USART2->DR = c;                    // Hardware will transmit this byte
}

void UART2_SendString(char *str) {
    // Send each character in the string until null terminator
    while (*str) {                     // While current character is not '\0'
        UART2_SendChar(*str++);        // Send char, then increment pointer
    }
}

void delay(uint32_t count) {
    // Simple blocking delay loop
    for (uint32_t i = 0; i < count; i++);  // Do nothing, just burn CPU cycles
}

int main(void) {
    // Initialize peripherals
    UART2_Init();   // Set up UART for transmission
    LED_Init();     // Set up LED on PA5

    // Main loop
    while (1) {
        UART2_SendString("Hello\n");   // Send string via UART (invisible until you connect serial)
        LED_Toggle();                   // Blink LED (visible confirmation code is running)
        delay(500000);                  // Wait ~500ms (rough timing)
    }
}
