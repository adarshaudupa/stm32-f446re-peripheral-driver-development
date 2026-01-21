#include "stm32f4xx.h"
#include "uart.h"

int main(void) {
    UART2_Init(9600);

    UART2_SendString("=== Interrupt UART Test ===\r\n");
    UART2_SendString("Type characters:\r\n\r\n");

    while (1) {
        char c = UART2_ReadChar();
        UART2_SendChar(c);
        if (c == '\r') {
            UART2_SendChar('\n');
        }
    }
}
