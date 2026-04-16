/*
 * hc-sr04.h
 *
 *  Created on: 18-Mar-2026
 *      Author: Adarsha Udupa
 */

#ifndef INC_HC_SR04_H_
#define INC_HC_SR04_H_

void HCSR04_Init(void);
uint32_t HCSR04_ReadPulseUs(void);
void HCSR04_Trigger(void);
static inline uint32_t HCSR04_Echo_Read(void);
uint32_t HCSR04_MeasureUs(void);

#endif /* INC_HC_SR04_H_ */
