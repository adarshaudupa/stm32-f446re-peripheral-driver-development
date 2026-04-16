/*
 * i2c1.c
 *
 *  Created on: Feb 28, 2026
 *      Author: Adarsha Udupa
 */
#include "i2c1.h"
#include "stm32f4xx.h"
#include "uart2.h"
#include "clock.h"

void I2C1_Init(uint32_t speed_hz)
{
 RCC->AHB1ENR |= (1<<1); //Enable GPIOB Clock
 RCC->APB1ENR |= (1<<21); //Enable I2C1 Clock
 GPIOB->MODER &= ~((3<<16) | (3<<18));
 GPIOB->MODER |= (2<<16) | (2<<18); //Setting PB8 and PB9 to AF
 GPIOB->OTYPER &= ~((1<<8) | (1<<9));
 GPIOB->OTYPER |= (1<<8) | (1<<9); //Set to Open drain type output
 GPIOB->OSPEEDR &= ~((3<<16) | (3<<18));
 GPIOB->OSPEEDR |= (3<<16) | (3<<18);
 GPIOB->PUPDR &= ~((3<<16) | (3<<18));
 GPIOB->PUPDR |= (1<<16) | (1<<18); //Set Internal Pull up resistor
 GPIOB->AFR[1] &= ~((0xF<<0) | (0xF<<4));
 GPIOB->AFR[1] |= (4<<0) | (4<<4); //PB8 and PB9 set to I2C AF(AF4)

 I2C1->CR1 = 0;
 I2C1->CR2 = 0;
 I2C1->OAR1 = 0;
 I2C1->OAR2 = 0;
 I2C1->CCR = 0;
 I2C1->TRISE = 0;

 uint32_t pclk = get_apb1_freq_hz();
 uint32_t pclk_mhz = pclk / 1000000;
 I2C1->CR2 = pclk_mhz;
 //CCR =(PClk/(2 or 3 * Target Frequency) 2 for standard mode(100KHz) and 3 for Fast mode(400Khz)
 I2C1->CCR = pclk / (2 * speed_hz);
 //TRISE = (PClk/1000000) +1 for Standard mode
 I2C1->TRISE = pclk_mhz + 1;
 I2C1->CR1 |= (1<<0); //PE i.e Peripheral Enable Bit Set

 uart_print_uint("I2C CR2 = ", I2C1->CR2);
 uart_print_uint("I2C CCR = ", I2C1->CCR);
 uart_print_uint("I2C TRISE = ", I2C1->TRISE);

}

uint8_t I2C1_ReadRegister(uint8_t dev_addr, uint8_t reg_addr)
{
    UART2_SendString("I2C: start\r\n");

    // 1. START
    I2C1->CR1 |= (1<<8); //START
    while (!(I2C1->SR1 & (1<<0))); //START bit set
    UART2_SendString("I2C: SB\r\n");

    // 2. Address + W
    I2C1->DR = (dev_addr << 1) | 0x00; //addr + W
    while (!(I2C1->SR1 & (1<<1))); //ADDR
    (void)I2C1->SR2; // clear ADDR
    UART2_SendString("I2C: ADDR-W\r\n");

    // 3. Reg addr
    I2C1->DR = reg_addr;
    while (!(I2C1->SR1 & (1<<7))); // Wait TXE=1 (reg addr sent)
    while (!(I2C1->SR1 & (1<<2))); // Wait BTF=1 (reg addr sent and ready for next byte)
    UART2_SendString("I2C: reg sent\r\n");

    // 4. Repeated START
    I2C1->CR1 |= (1<<8); //REPEATED START
    while (!(I2C1->SR1 & (1<<0))); //START bit set
    UART2_SendString("I2C: SB2\r\n");

    // 5. Address + R
    I2C1->DR = (dev_addr << 1) | 0x01; //addr + R
    while (!(I2C1->SR1 & (1<<1))); //ADDR set
    UART2_SendString("I2C: ADDR-R\r\n");

    I2C1->CR1 &= ~(1<<10);   // ACK = 0
    (void)I2C1->SR1;
    (void)I2C1->SR2;         // clear ADDR
    I2C1->CR1 |= (1<<9);      // STOP = 1

    while (!(I2C1->SR1 & (1<<6))); // Wait RXNE=1 (data received)
    UART2_SendString("I2C: RXNE\r\n");
    uint8_t data = I2C1->DR;
    I2C1->CR1 |= (1<<10); //re-enable ACK for next reads

    UART2_SendString("I2C: done\r\n");
    return data;
}



void I2C1_ReadMulti(uint8_t saddr, uint8_t maddr, uint8_t *data, uint8_t len)
{
    UART2_SendString("RM: enter\r\n");

    // 1. Wait until bus not busy
    // while (I2C1->SR2 & I2C_SR2_BUSY);

    // 2. Generate START
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));
    UART2_SendString("RM: SB set\r\n");

    // 3. Send slave address (WRITE)
    I2C1->DR = saddr << 1;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    UART2_SendString("RM: ADDR write\r\n");
    (void)I2C1->SR2;

    // 4. Send register address
    while (!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = maddr;
    while (!(I2C1->SR1 & I2C_SR1_TXE));
    UART2_SendString("RM: reg sent\r\n");

    // 5. Repeated START
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));
    UART2_SendString("RM: SB2 set\r\n");

    // 6. Send slave address (READ)
    I2C1->DR = (saddr << 1) | 0x01;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    UART2_SendString("RM: ADDR read\r\n");
    (void)I2C1->SR2;

    // 7. One-byte debug read
    I2C1->CR1 &= ~I2C_CR1_ACK;
    I2C1->CR1 |= I2C_CR1_STOP;
    while (!(I2C1->SR1 & I2C_SR1_RXNE));
    data[0] = I2C1->DR;
    I2C1->CR1 |= I2C_CR1_ACK;

    UART2_SendString("RM: exit\r\n");
}

void I2C1_WriteByte(uint8_t dev_addr, uint8_t reg_addr, uint8_t value)
{
    // 1. START condition
    I2C1->CR1 |= (1 << 8);                      // START
    while (!(I2C1->SR1 & (1 << 0)));           // Wait SB=1

    // 2. Send slave address + WRITE (W = 0)
    I2C1->DR = (dev_addr << 1) | 0x00;
    while (!(I2C1->SR1 & (1 << 1)));           // Wait ADDR=1
    (void)I2C1->SR2;                           // Clear ADDR

    // 3. Send register address
    while (!(I2C1->SR1 & (1 << 7)));           // Wait TXE=1
    I2C1->DR = reg_addr;
    while (!(I2C1->SR1 & (1 << 2)));           // Wait BTF=1 (reg addr sent)

    // 4. Send data byte
    I2C1->DR = value;
    while (!(I2C1->SR1 & (1 << 2)));           // Wait BTF=1 (data sent)

    // 5. STOP condition
    I2C1->CR1 |= (1 << 9);                     // STOP
}
