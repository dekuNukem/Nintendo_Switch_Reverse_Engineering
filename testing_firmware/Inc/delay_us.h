#ifndef __DELAY_US_H
#define __DELAY_US_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f0xx_hal.h"

void delay_us_init(TIM_HandleTypeDef* htim_base);
void delay_us(uint32_t delay);
uint32_t micros(void);

#ifdef __cplusplus
}
#endif

#endif


