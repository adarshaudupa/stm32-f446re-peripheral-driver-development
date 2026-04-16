/*
 * bme280.c
 *
 *  Created on: Mar 3, 2026
 *      Author: Adarsha Udupa
 */


#include "bme280.h"
#include "i2c1.h"

#define BME280_ADDR   0x76
#define BME280_WHO_AM_I_REG  0xD0
#define BME280_CHIP_ID       0x60

uint8_t BME280_CheckID(void)
{
    return I2C1_ReadRegister(BME280_ADDR, BME280_WHO_AM_I_REG);
}

void BME280_ReadRawTemp(uint8_t *buf)
{
    I2C1_ReadMulti(BME280_ADDR, 0xFA, buf, 3);
}
