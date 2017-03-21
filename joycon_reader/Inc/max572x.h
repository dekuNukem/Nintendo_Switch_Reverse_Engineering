#ifndef __MAX572X_H
#define __MAX572X_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f0xx_hal.h"
#include "shared.h"

#define max572x_POWER_NORMAL 0x0
#define max572x_POWER_1K 0x40
#define max572x_POWER_100K 0x80
#define max572x_POWER_HIZ 0xc0

#define max572x_CONFIG_WDOG_DISABLE 0x0
#define max572x_CONFIG_WDOG_GATE 0x40
#define max572x_CONFIG_WDOG_CLR 0x80
#define max572x_CONFIG_WDOG_HOLD 0xc0
#define max572x_CONFIG_GATE_EN 0x20
#define max572x_CONFIG_LDAC_EN 0x10
#define max572x_CONFIG_CLR_EN 0x8

#define max572x_WDOG_WD_MASK 0x8
#define max572x_WDOG_SAFETY_LOW 0x0
#define max572x_WDOG_SAFETY_MID 0x2
#define max572x_WDOG_SAFETY_HIGH 0x4
#define max572x_WDOG_SAFETY_MAX 0x6

#define max572x_REF_POWER 0x4
#define max572x_REF_EXT 0x0
#define max572x_REF_2V5 0x1
#define max572x_REF_2V0 0x2
#define max572x_REF_4V0 0x3

void max572x_SW_RESET(void);
void max572x_POWER(uint8_t dac_multi_sel, uint8_t power_mode);
void max572x_CONFIG(uint8_t dac_multi_sel, uint8_t wdog, uint8_t gate_en, uint8_t ldac_en, uint8_t clear_en);
void max572x_WDOG(uint16_t wd_timeout, uint8_t wd_mask, uint8_t wd_safety);
void max572x_REF(uint8_t ref_power, uint8_t ref_mode);
void max572x_CODEn(uint8_t dac_sel, uint16_t dac_value);
void max572x_LOADn(uint8_t dac_sel);
void max572x_CODEn_LOAD_ALL(uint8_t dac_sel, uint16_t dac_value);
void max572x_CODEn_LOADn(uint8_t dac_sel, uint16_t dac_value);
void max572x_CODE_ALL(uint16_t dac_value);
void max572x_LOAD_ALL(void);
void max572x_CODE_ALL_LOAD_ALL(uint16_t dac_value);
void max_init(void);
void max_test(void);
void stm_dac_test(void);

#ifdef __cplusplus
}
#endif

#endif


