/*
 * bme280.h
 *
 *  Created on: Mar 3, 2026
 *      Author: Adarsha Udupa
 */
#include <stdint.h>
#include "i2c1.h"
#ifndef INC_BME280_H_
#define INC_BME280_H_

uint8_t BME280_CheckID(void);
void BME280_ReadRawTemp(uint8_t *buf);

#endif /* INC_BME280_H_ */
