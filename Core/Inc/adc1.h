/*
 * adc.h
 *
 *  Created on: Feb 24, 2026
 *      Author: Adarsha Udupa
 */

#ifndef SRC_ADC_H_
#define SRC_ADC_H_
#include "stm32f4xx.h"
#include <stdlib.h>
void ADC1_Init(void);
uint8_t ADC1_Read();

#endif /* SRC_ADC_H_ */
