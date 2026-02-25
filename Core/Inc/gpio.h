/*
 * gpio.h
 *
 *  Created on: Feb 25, 2026
 *      Author: Adarsha Udupa
 */

#ifndef INC_GPIO_H_
#define INC_GPIO_H_
#include <stdint.h>
void PC13_Init(void);
void LED_ON(void);
void LED_OFF(void);
void LED_Toggle(void);
void PA5_Init(void);
uint8_t Button_Read(void);

#endif /* INC_GPIO_H_ */
