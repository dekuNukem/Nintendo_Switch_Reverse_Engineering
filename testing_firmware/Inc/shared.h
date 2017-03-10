#ifndef __shared_H
#define __shared_H

#include "stm32f0xx_hal.h"
#include "main.h"

#define HIGH GPIO_PIN_SET
#define LOW GPIO_PIN_RESET

extern DAC_HandleTypeDef hdac;
#define stm32_dac_ptr (&hdac)

extern IWDG_HandleTypeDef hiwdg;
#define stm32_iwdg_ptr (&hiwdg)

extern I2C_HandleTypeDef hi2c2;
#define stm32_i2c_ptr (&hi2c2)

extern int32_t next_iwdg_kick;
extern int32_t board_type;

#endif
