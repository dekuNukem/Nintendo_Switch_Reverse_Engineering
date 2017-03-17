/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "helpers.h"
#include "shared.h"
#include "max572x.h"
#include "my_usb.h"
#include "cmd_parser.h"
#include "delay_us.h"

#define NSTX_ENABLE() HAL_GPIO_WritePin(FC_NS_TX_EN_GPIO_Port, FC_NS_TX_EN_Pin, GPIO_PIN_RESET)
#define NSTX_DISABLE() HAL_GPIO_WritePin(FC_NS_TX_EN_GPIO_Port, FC_NS_TX_EN_Pin, GPIO_PIN_SET)
#define UART_WAITING 0
#define UART_RECEIVED 1
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac;

IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
int32_t next_iwdg_kick;
uint8_t handshake_header[4] = {0xa1, 0xa2, 0xa3, 0xa4};
linear_buf switch_lb;
volatile uint8_t uart_status;

uint8_t connect_request[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0xA5, 0x02, 0x01, 0x7E, 0x00, 0x00, 0x00};
uint8_t connect_request_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0xA5, 0x02, 0x02, 0x7D, 0x00, 0x00, 0x64};

uint8_t cmd2[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x01, 0x00, 0x00, 0x00, 0x00, 0x24};
// grey
uint8_t cmd2_response[20] = {0x19, 0x81, 0x03, 0x0F, 0x00, 0x94, 0x01, 0x08, 0x00, 0x00, 0xFA, 0xE8, 0x01, 0x31, 0x67, 0x9C, 0x8A, 0xBB, 0x7C, 0x00};
// red
// uint8_t cmd2_response[20] = {0x19, 0x81, 0x03, 0x0F, 0x00, 0x94, 0x01, 0x08, 0x00, 0x00, 0x8F, 0x87, 0x01, 0xE6, 0x4C, 0x5F, 0xB9, 0xE6, 0x98, 0x00};

uint8_t cmd3[20] = {0x19, 0x01, 0x03, 0x0F, 0x00, 0x91, 0x20, 0x08, 0x00, 0x00, 0xBD, 0xB1, 0xC0, 0xC6, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t cmd3_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x20, 0x00, 0x00, 0x00, 0x00, 0xA8};

uint8_t cmd4[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x11, 0x00, 0x00, 0x00, 0x00, 0x0E};
uint8_t cmd4_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x11, 0x00, 0x00, 0x0F, 0x00, 0x33};

uint8_t cmd5[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x10, 0x00, 0x00, 0x00, 0x00, 0x3D};
uint8_t cmd5_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x10, 0x00, 0x00, 0x00, 0x00, 0xD6};

uint8_t cmd6[16] = {0x19, 0x01, 0x03, 0x0B, 0x00, 0x91, 0x12, 0x04, 0x00, 0x00, 0x12, 0xA6, 0x0F, 0x00, 0x00, 0x00};
uint8_t cmd6_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x12, 0x00, 0x00, 0x00, 0x00, 0xB0};

uint8_t update_request[13] = {0x19, 0x01, 0x03, 0x08, 0x00, 0x92, 0x00, 0x01, 0x00, 0x00, 0x69, 0x2D, 0x1F};
uint8_t update_request_reponse[61] = {0x19, 0x81, 0x03, 0x38, 0x00, 0x92, 0x00, 0x31, 0x00, 0x00, 0xD2, 0xD2, 0x30, 0xCA, 0x50, 0x00, 0x80, 0x00, 0x6F, 0x67, 0x84, 0x00, 0x00, 0x00, 0x90, 0x4D, 0x00, 0x2E, 0x00, 0x71, 0x10, 0xF7, 0xFF, 0xDD, 0xFF, 0xA5, 0x00, 0x9B, 0x00, 0x49, 0x00, 0x4A, 0x10, 0x36, 0x00, 0xD9, 0xFF, 0xA7, 0x00, 0x05, 0x01, 0x56, 0x00, 0xA3, 0x10, 0x3E, 0x00, 0xDE, 0xFF, 0xA7, 0x00};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
static void MX_DAC_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void stm32_dac_init(void)
{
  MX_DAC_Init();
  HAL_DAC_Start(stm32_dac_ptr, DAC_CHANNEL_1);
  HAL_DAC_Start(stm32_dac_ptr, DAC_CHANNEL_2);
}

int fputc(int ch, FILE *f)
{
  my_usb_putchar((uint8_t)ch);
  return ch;
}

void blink(void)
{
  for (int i = 0; i < 8; ++i)
  {
    HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
    HAL_Delay(100);
  }
}

void fatal_error()
{
  while(1)
  {
    HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_SET);
    HAL_Delay(400);
    HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);

    for (int i = 0; i < 32; ++i)
    printf("0x%x ", switch_lb.buf[i]);
    printf("\n\n");
  }
}

void usart2_init_3125000(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 3125000;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXINVERT_INIT;
  huart2.AdvancedInit.RxPinLevelInvert = UART_ADVFEATURE_RXINV_ENABLE;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

void transmit_with_cts(uint8_t *data, uint16_t size)
{
  for (int i = 0; i < size; ++i)
  {
    while(HAL_GPIO_ReadPin(GPIOA, FC_JC_TX_EN_Pin) == GPIO_PIN_RESET)
      ;
    HAL_UART_Transmit(&huart2, data + i, 1, 1000);
  }
}

void get_header()
{
  HAL_UART_Receive_IT(&huart2, switch_lb.buf, 4);
  NSTX_ENABLE();
  uart_status = UART_WAITING;
  while(uart_status != UART_RECEIVED)
    if(huart2.RxXferCount <= 1)
      NSTX_DISABLE();
}

void get_payload()
{
  uint8_t* buf_start = switch_lb.buf + 4;
  uint8_t recv_size = switch_lb.buf[3] + 1;
  HAL_UART_Receive_IT(&huart2, buf_start, recv_size);
  NSTX_ENABLE();
  uart_status = UART_WAITING;
  while(uart_status != UART_RECEIVED)
    if(huart2.RxXferCount <= 1)
      NSTX_DISABLE();
}

void get_msg()
{
  linear_buf_reset(&switch_lb);
  get_header();
  if(memcmp(switch_lb.buf, handshake_header, 4) == 0)
    get_header();
  get_payload();
}

void attach()
{
  HAL_UART_MspDeInit(&huart2);
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_Delay(1000);
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);
  HAL_UART_MspInit(&huart2);
  MX_USART2_UART_Init();

  get_msg();
  if(memcmp(switch_lb.buf, connect_request, 12) != 0)
    fatal_error();
  transmit_with_cts(connect_request_response, 12);
  
  get_msg();
  if(memcmp(switch_lb.buf, cmd2, 12) != 0)
    fatal_error();
  transmit_with_cts(cmd2_response, 20);

  get_msg();
  if(memcmp(switch_lb.buf, cmd3, 20) != 0)
    fatal_error();
  transmit_with_cts(cmd3_response, 12);

  HAL_UART_MspDeInit(&huart2);
  while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET)
    ;
  while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET)
    ;
  HAL_UART_MspInit(&huart2);
  usart2_init_3125000();

  get_msg();
  if(memcmp(switch_lb.buf, cmd4, 12) != 0)
    fatal_error();
  transmit_with_cts(cmd4_response, 12);

  get_msg();
  if(memcmp(switch_lb.buf, cmd5, 12) != 0)
    fatal_error();
  transmit_with_cts(cmd5_response, 12);

  get_msg();
  if(memcmp(switch_lb.buf, cmd6, 16) != 0)
    fatal_error();
  transmit_with_cts(cmd6_response, 12);

  while(1)
  {
    get_msg();
    if(memcmp(switch_lb.buf, update_request, 13) == 0)
      transmit_with_cts(update_request_reponse, 61);
  }

  for (int i = 0; i < 32; ++i)
    printf("0x%x ", switch_lb.buf[i]);
  printf("\ndone!\n");

}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
  my_usb_init();
  next_iwdg_kick = HAL_GetTick() + 500;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_USB_DEVICE_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();

  /* USER CODE BEGIN 2 */
  NSTX_ENABLE();
  linear_buf_reset(&switch_lb);
  blink();
  // spi_cs_high();
  delay_us_init(&htim2);
  uart_status = UART_RECEIVED;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    if(HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET)
    {
      HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_RESET);
      printf("%d\n", HAL_GetTick());
      attach();
      HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_SET);
    }
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48
                              |RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL7;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* DAC init function */
static void MX_DAC_Init(void)
{

  DAC_ChannelConfTypeDef sConfig;

    /**DAC Initialization 
    */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

    /**DAC channel OUT1 config 
    */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

    /**DAC channel OUT2 config 
    */
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

}

/* IWDG init function */
static void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_16;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }

}

/* SPI2 init function */
static void MX_SPI2_Init(void)
{

  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 55;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 1000000;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXINVERT_INIT;
  huart2.AdvancedInit.RxPinLevelInvert = UART_ADVFEATURE_RXINV_ENABLE;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(FC_NS_TX_EN_GPIO_Port, FC_NS_TX_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : FC_JC_TX_EN_Pin JOYCON_CTS_Pin */
  GPIO_InitStruct.Pin = FC_JC_TX_EN_Pin|JOYCON_CTS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : FC_NS_TX_EN_Pin */
  GPIO_InitStruct.Pin = FC_NS_TX_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(FC_NS_TX_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI2_CS_Pin */
  GPIO_InitStruct.Pin = SPI2_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI2_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DEBUG_LED_Pin */
  GPIO_InitStruct.Pin = DEBUG_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DEBUG_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USER_BUTTON_Pin */
  GPIO_InitStruct.Pin = USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  NSTX_DISABLE();
  HAL_UART_AbortReceive(huart);
  uart_status = UART_RECEIVED;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  ;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
