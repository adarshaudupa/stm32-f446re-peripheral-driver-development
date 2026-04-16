#include "stm32f4xx.h"
#include "uart2.h"
#include "i2c1.h"
#include "gpio.h"
#include "adc1.h"
#include "tim2.h"
#include "clock.h"
#include <stdint.h>


int main(void)
{
    UART2_Init();
    I2C1_Init(100000);
    UART2_SendString("I2C init done\r\n");

    uint8_t id = I2C1_ReadRegister(0x76, 0xD0);  // LSM6DS3: addr=0x76, WHO_AM_I reg=0xD0
    uart_print_uint("WHO_AM_I = ", id);            // expect 0x60

    while (1) {}
}


