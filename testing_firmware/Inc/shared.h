#ifndef __shared_H
#define __shared_H

#include "stm32f0xx_hal.h"
#include "main.h"

#define HIGH GPIO_PIN_SET
#define LOW GPIO_PIN_RESET

extern SPI_HandleTypeDef hspi2;
#define max572x_spi_ptr (&hspi2)

extern DAC_HandleTypeDef hdac;
#define stm32_dac_ptr (&hdac)

extern IWDG_HandleTypeDef hiwdg;
#define stm32_iwdg_ptr (&hiwdg)

extern UART_HandleTypeDef huart2;
#define stm32_uart2_ptr (&huart2)

extern int32_t next_iwdg_kick;
extern volatile uint8_t uart_status;

#define spi_cs_low() do { HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET); } while (0)
#define spi_cs_high() do { HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET); } while (0)

#endif
