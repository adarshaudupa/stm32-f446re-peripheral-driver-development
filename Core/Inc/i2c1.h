/*
 * i2c1.h
 *
 *  Created on: Feb 28, 2026
 *      Author: Adarsha Udupa
 */
#include <stdint.h>
#ifndef INC_I2C1_H_
#define INC_I2C1_H_

void I2C1_Init(uint32_t speed_hz);
uint8_t I2C1_ReadRegister(uint8_t dev_addr, uint8_t reg_addr);
void I2C1_ReadMulti(uint8_t slave_addr, uint8_t reg_addr,uint8_t *buf, uint8_t len);
void I2C1_WriteByte(uint8_t dev_addr, uint8_t reg_addr, uint8_t value);

#endif /* INC_I2C1_H_ */
