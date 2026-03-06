#include "stm32f4xx.h"
#include "uart2.h"
#include "i2c1.h"
#include "gpio.h"
#include "adc1.h"
#include "tim2.h"
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


int main(void)
{
   UART2_Init();
   PA5_Init();
   TIM2_Init();
   ADC1_Init();
   I2C1_Init();
   uint8_t id = 0;

   id = I2C1_ReadRegister(0x76, 0xD0);
   if(id == 0xFF)
   {
	UART2_SendString("Sensor Read is okay\r\n");
   }
   else
	   UART2_SendString("Sensor Read fail\r\n");

   UART2_SendString("STM32 Peripheral Demo\r\n");
    while (1)
    {
     if(tim2_flag)
     {
       tim2_flag = 0;
       uint16_t adc = ADC1_Read();
          if(adc>2095)
       	 LED_ON();
          else
       	   LED_OFF();

          char buff[16];
          uint16_to_string(adc, buff);
          UART2_SendString("ADC = ");
          UART2_SendString(buff);
          UART2_SendString("\r\n");
     }
    }
}
