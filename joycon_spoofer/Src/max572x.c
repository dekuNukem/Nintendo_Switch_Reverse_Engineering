#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "helpers.h"
#include "shared.h"
#include "max572x.h"

// SPI_POLARITY_LOW is a must
// if SPI_PHASE_1EDGE, DPHA = 0
// if SPI_PHASE_2EDGE, DPHA = 1

void max572x_SW_RESET(void)
{
  uint8_t max572x_cmd_SW_RESET[3] = {0x35, 0x96, 0x30};
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, max572x_cmd_SW_RESET, 3, 100);
  spi_cs_high();
}

/*
  dac_multi_sel: each bit selects a coresponding DAC
  power_mode: one of the max572x_POWER_XXX defines in the header file
*/
void max572x_POWER(uint8_t dac_multi_sel, uint8_t power_mode)
{
  uint8_t spi_sb[3] = {0x40, dac_multi_sel, power_mode};
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
}

/*
  dac_multi_sel: each bit selects a coresponding DAC
  wdog: look at datasheet page 26 for details
  gate_en: 0 enable software gating, 1 disable
  ldac_en: 0 DAC latch enabled, 1 DAC latch transparent
  clear_en: 0 clear input and command works, 1 clear has no effect
*/
void max572x_CONFIG(uint8_t dac_multi_sel, uint8_t wdog, uint8_t gate_en, uint8_t ldac_en, uint8_t clear_en)
{
  uint8_t spi_sb[3] = {0x50, dac_multi_sel, 0};
  spi_sb[2] |= wdog;
  if(gate_en)
    spi_sb[2] |= max572x_CONFIG_GATE_EN;
  if(ldac_en)
    spi_sb[2] |= max572x_CONFIG_LDAC_EN;
  if(clear_en)
    spi_sb[2] |= max572x_CONFIG_CLR_EN;
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
}

/*
  wd_timeout: 12-bit timeout value in ms, max 4096ms
  wd_mask: when 1 timeout will not assert IRQ
  wd_safety: one of the max572x_WDOG_SAFETY_XXX defines in header file
*/
void max572x_WDOG(uint16_t wd_timeout, uint8_t wd_mask, uint8_t wd_safety)
{
  uint8_t spi_sb[3] = {0x10, 0, 0};
  wd_timeout &= 0xfff;
  spi_sb[1] |= wd_timeout >> 4;
  wd_timeout &= 0xf;
  spi_sb[2] |= (wd_timeout << 4);
  if(wd_mask)
    spi_sb[2] |= max572x_WDOG_WD_MASK;
  spi_sb[2] |= wd_safety;
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
}

/*
  ref_power: 0 auto power off, 1 always on
  ref_mode: one of the max572x_REF_XXX defines in header file
*/
void max572x_REF(uint8_t ref_power, uint8_t ref_mode)
{
  uint8_t spi_sb[3] = {0x20, 0, 0};
  if(ref_power)
    spi_sb[0] |= max572x_REF_POWER;
  spi_sb[0] |= ref_mode;
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
}

void pack_dac_data(uint16_t value, uint8_t* dh, uint8_t* dl)
{
  value &= 0x3ff;
    *dh |= value >> 2;
    value &= 0x3;
    *dl |= (value << 6);
}

void max572x_CODEn(uint8_t dac_sel, uint16_t dac_value)
{
  uint8_t spi_sb[3] = {0x80, 0, 0};
  dac_sel &= 0xf;
  spi_sb[0] |= dac_sel;
  pack_dac_data(dac_value, &spi_sb[1], &spi_sb[2]);
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
}

void max572x_LOADn(uint8_t dac_sel)
{
  uint8_t spi_sb[3] = {0x90, 0, 0};
  dac_sel &= 0xf;
  spi_sb[0] |= dac_sel;
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
}

void max572x_CODEn_LOAD_ALL(uint8_t dac_sel, uint16_t dac_value)
{
  uint8_t spi_sb[3] = {0xa0, 0, 0};
  dac_sel &= 0xf;
  spi_sb[0] |= dac_sel;
  pack_dac_data(dac_value, &spi_sb[1], &spi_sb[2]);
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
}

void max572x_CODEn_LOADn(uint8_t dac_sel, uint16_t dac_value)
{
  uint8_t spi_sb[3] = {0xb0, 0, 0};
  dac_sel &= 0xf;
  spi_sb[0] |= dac_sel;
  pack_dac_data(dac_value, &spi_sb[1], &spi_sb[2]);
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
}

void max572x_CODE_ALL(uint16_t dac_value)
{
  uint8_t spi_sb[3] = {0xc0, 0, 0};
  pack_dac_data(dac_value, &spi_sb[1], &spi_sb[2]);
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
}

void max572x_LOAD_ALL(void)
{
  uint8_t spi_sb[3] = {0xc1, 0, 0};
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
}

void max572x_CODE_ALL_LOAD_ALL(uint16_t dac_value)
{
  uint8_t spi_sb[3] = {0xc2, 0, 0};
  pack_dac_data(dac_value, &spi_sb[1], &spi_sb[2]);
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
}

void max_init(void)
{
  max572x_SW_RESET();
  max572x_CONFIG(0xff, max572x_CONFIG_WDOG_DISABLE, 1, 0, 0);
  max572x_WDOG(1000, 1, max572x_WDOG_SAFETY_LOW);
  max572x_REF(1, max572x_REF_EXT);
  max572x_POWER(0xff, max572x_POWER_HIZ);
}

void max_test(void)
{
  max_init();
  max572x_POWER(0x3, max572x_POWER_NORMAL);
  printf("run started\n");
  uint16_t count = 0;
  while(1)
  {
    max572x_CODEn_LOADn(0, count);
    max572x_CODEn_LOADn(1, ~count);
    count++;
  }
}

void stm_dac_test(void)
{
  stm32_dac_init();
  printf("run started\n");
  uint16_t count = 0;
  while(1)
  {
    HAL_DACEx_DualSetValue(stm32_dac_ptr, DAC_ALIGN_12B_R, count, 65535 - count);
    count++;
  }
}
