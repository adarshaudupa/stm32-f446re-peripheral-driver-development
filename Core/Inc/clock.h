/*
 * clock.h
 *
 *  Created on: Mar 9, 2026
 *      Author: Adarsha Udupa
 */

#ifndef INC_CLOCK_H_
#define INC_CLOCK_H_

#include <stdint.h>

uint32_t get_sysclk_freq_hz(void);
//uint32_t get_ahb_freq_hz(void);
uint32_t get_apb1_freq_hz(void);
uint32_t get_apb2_freq_hz(void);


#endif /* INC_CLOCK_H_ */
