#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "joycon_cmd.h"

linear_buf switch_lb;

uint8_t handshake_header[4] = {0xa1, 0xa2, 0xa3, 0xa4};
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

void usart2_init(uint32_t baud)
{
  stm32_uart2_ptr->Instance = USART2;
  stm32_uart2_ptr->Init.BaudRate = baud;
  stm32_uart2_ptr->Init.WordLength = UART_WORDLENGTH_8B;
  stm32_uart2_ptr->Init.StopBits = UART_STOPBITS_1;
  stm32_uart2_ptr->Init.Parity = UART_PARITY_NONE;
  stm32_uart2_ptr->Init.Mode = UART_MODE_TX_RX;
  stm32_uart2_ptr->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  stm32_uart2_ptr->Init.OverSampling = UART_OVERSAMPLING_16;
  stm32_uart2_ptr->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  stm32_uart2_ptr->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXINVERT_INIT;
  stm32_uart2_ptr->AdvancedInit.RxPinLevelInvert = UART_ADVFEATURE_RXINV_ENABLE;
  if (HAL_UART_Init(stm32_uart2_ptr) != HAL_OK)
  {
    fatal_error();
  }
}

void transmit_with_cts(uint8_t *data, uint16_t size)
{
  for (int i = 0; i < size; ++i)
  {
    while(HAL_GPIO_ReadPin(GPIOA, FC_JC_TX_EN_Pin) == GPIO_PIN_RESET)
      ;
    HAL_UART_Transmit(stm32_uart2_ptr, data + i, 1, 1000);
  }
}

void get_header()
{
  HAL_UART_Receive_IT(stm32_uart2_ptr, switch_lb.buf, 4);
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
  HAL_UART_Receive_IT(stm32_uart2_ptr, buf_start, recv_size);
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
  HAL_UART_MspDeInit(stm32_uart2_ptr);
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_Delay(1000);
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);
  HAL_UART_MspInit(stm32_uart2_ptr);
  usart2_init(1000000);

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

  HAL_UART_MspDeInit(stm32_uart2_ptr);
  while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET)
    ;
  while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET)
    ;
  HAL_UART_MspInit(stm32_uart2_ptr);
  usart2_init(3125000);

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

