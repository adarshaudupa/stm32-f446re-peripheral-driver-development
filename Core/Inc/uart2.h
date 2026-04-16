/*
 * uart.h
 *
 * Minimal UART driver for STM32F446RE
 * USART2: PA2 (TX), PA3 (RX)
 *
 * Register-level implementation (no HAL)
 */

#ifndef INC_UART2_H__
#define INC_UART2_H_H_

#include<stdint.h>
// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

/**
 * @brief  Initialize USART2 for TX and RX
 * @param  baudrate: Desired baud rate (e.g., 9600, 115200)
 * @note   Configures PA2 (TX) and PA3 (RX) as AF7
 * @note   Assumes 16 MHz APB1 clock
 */
void UART2_Init(void);
/**
 * @brief  Transmit a single character (blocking)
 * @param  ch: Character to send
 * @note   Waits for TXE flag before writing to DR
 */
void UART2_SendChar(char ch);

/**
 * @brief  Transmit a null-terminated string (blocking)
 * @param  str: Pointer to string
 */
void UART2_SendString(const char *str);

/**
 * @brief  Receive a single character (blocking)
 * @return Received character
 * @note   Waits for RXNE flag before reading from DR
 */
char UART2_ReadChar(void);

/**
 * @brief  Check if data is available to read
 * @return 1 if data ready, 0 if not
 * @note   Non-blocking check of RXNE flag
 */
uint8_t UART2_DataAvailable(void);

void uart_print_uint(const char *label, uint32_t value);

#endif /*INC_UART2_H__H_ */
