#include "stm32f4xx.h"
#include "uart2.h"
#include "i2c1.h"
#include "gpio.h"
#include "adc1.h"
#include "tim2.h"
#include "clock.h"
#include <stdint.h>

void uint16_to_string(uint16_t num, char *str)
{
    int i = 0;

    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    while (num > 0) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    str[i] = '\0';

    // reverse
    for (int j = 0; j < i / 2; j++) {
        char tmp = str[j];
        str[j] = str[i - 1 - j];
        str[i - 1 - j] = tmp;
    }
}


static void uart_print_uint(const char *label, uint32_t value) {
    char buf[16];
    int idx = 0;
    if (value == 0) {
        buf[idx++] = '0';
    } else {
        char tmp[16];
        int t = 0;
        while (value > 0 && t < 16) {
            tmp[t++] = '0' + (value % 10);
            value /= 10;
        }
        while (t > 0) {
            buf[idx++] = tmp[--t];
        }
    }
    buf[idx] = '\0';

    UART2_SendString(label);
    UART2_SendString(buf);
    UART2_SendString("\r\n");
}

int main(void) {
    UART2_Init(115200);
    PA5_Init();
    TIM2_Init();
    uint32_t cfgr = RCC->CFGR;
    uint32_t sys  = get_sysclk_freq_hz();
    uint32_t apb1 = get_apb1_freq_hz();

    uart_print_uint("CFGR = ", cfgr);
    uart_print_uint("SYS  = ", sys);
    uart_print_uint("APB1 = ", apb1);

    timer_start();
    
    while (1)
    {

    }
}
