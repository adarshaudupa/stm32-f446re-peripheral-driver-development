#include "stm32f4xx.h"
#include "gpio.h"
#include "uart2.h"
#include "tim2.h"
#include "adc1.h"
#include <stdint.h>

extern volatile uint8_t tim2_flag;

static void uint16_to_str(uint16_t val, char *buf) {
    int i = 0;
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
    buf[i] = '\0';
    // reverse
    for (int a = 0, b = i-1; a < b; a++, b--) {
        char t = buf[a]; buf[a] = buf[b]; buf[b] = t;
    }
}

int main(void) {
    PA5_Init();
    PC13_Init();
    UART2_Init(115200);
    TIM2_Init();
    ADC1_Init();

    UART2_SendString("Peripheral driver test\r\n");

    while (1) {
        if (tim2_flag) {
            tim2_flag = 0;

            ADC1->CR2 |= (1 << 30);
            while (!(ADC1->SR & (1 << 1)));
            uint16_t adc_val = ADC1->DR;

            char buf[8];
            uint16_to_str(adc_val, buf);
            UART2_SendString("ADC: ");
            UART2_SendString(buf);
            UART2_SendString("\r\n");

            if (adc_val > 2000) LED_On();
            else                LED_Off();
        }
    }
}
