#ifndef __JOYCON_CMD_H
#define __JOYCON_CMD_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f0xx_hal.h"
#include "main.h"
#include "helpers.h"
#include "shared.h"

#define NSTX_ENABLE() HAL_GPIO_WritePin(FC_NS_TX_EN_GPIO_Port, FC_NS_TX_EN_Pin, GPIO_PIN_RESET)
#define NSTX_DISABLE() HAL_GPIO_WritePin(FC_NS_TX_EN_GPIO_Port, FC_NS_TX_EN_Pin, GPIO_PIN_SET)

#define JCTX_ENABLE() HAL_GPIO_WritePin(FC_JC_TX_EN_GPIO_Port, FC_JC_TX_EN_Pin, GPIO_PIN_SET)
#define JCTX_DISABLE() HAL_GPIO_WritePin(FC_JC_TX_EN_GPIO_Port, FC_JC_TX_EN_Pin, GPIO_PIN_RESET)

#define UART_WAITING 0
#define UART_RECEIVED 1

void read_joycon(void);

#ifdef __cplusplus
}
#endif

#endif


