#ifndef __EEPROM_H
#define __EEPROM_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f0xx_hal.h"
#include "helpers.h"
#include "shared.h"

#define EEPROM_BOARD_TYPE_ADDR 0
#define EEPROM_BUTTON_RELEASE_DURATION_MS_ADDR 1
#define EEPROM_STICK_RELEASE_DURATION_MS_ADDR 2

void eeprom_init(I2C_HandleTypeDef *hi2c);
void eeprom_write(uint16_t address, uint8_t data);
uint8_t eeprom_read(int32_t address);
void eeprom_erase(void);

#ifdef __cplusplus
}
#endif

#endif


