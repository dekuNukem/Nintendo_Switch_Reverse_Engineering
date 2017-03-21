#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "joycon_cmd.h"

linear_buf switch_lb;

uint8_t handshake_header[4] = {0xa1, 0xa2, 0xa3, 0xa4};
uint8_t connect_request[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0xA5, 0x02, 0x01, 0x7E, 0x00, 0x00, 0x00};
uint8_t connect_request_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0xA5, 0x02, 0x02, 0x7D, 0x00, 0x00, 0x64};

uint8_t cmd2[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x01, 0x00, 0x00, 0x00, 0x00, 0x24};
uint8_t cmd2_response[20] = {0x19, 0x81, 0x03, 0x0F, 0x00, 0x94, 0x01, 0x08, 0x00, 0x00, 0xFA, 0xE8, 0x01, 0x31, 0x67, 0x9C, 0x8A, 0xBB, 0x7C, 0x00};

uint8_t cmd3[20] = {0x19, 0x01, 0x03, 0x0F, 0x00, 0x91, 0x20, 0x08, 0x00, 0x00, 0xBD, 0xB1, 0xC0, 0xC6, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t cmd3_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x20, 0x00, 0x00, 0x00, 0x00, 0xA8};

uint8_t cmd4[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x11, 0x00, 0x00, 0x00, 0x00, 0x0E};
uint8_t cmd4_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x11, 0x00, 0x00, 0x0F, 0x00, 0x33};

uint8_t cmd5[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x10, 0x00, 0x00, 0x00, 0x00, 0x3D};
uint8_t cmd5_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x10, 0x00, 0x00, 0x00, 0x00, 0xD6};

uint8_t cmd6[16] = {0x19, 0x01, 0x03, 0x0B, 0x00, 0x91, 0x12, 0x04, 0x00, 0x00, 0x12, 0xA6, 0x0F, 0x00, 0x00, 0x00};
uint8_t cmd6_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x12, 0x00, 0x00, 0x00, 0x00, 0xB0};

uint8_t update_request[13] = {0x19, 0x01, 0x03, 0x08, 0x00, 0x92, 0x00, 0x01, 0x00, 0x00, 0x69, 0x2D, 0x1F};

void fatal_error()
{
  while(1)
  {
    HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_SET);
    HAL_Delay(400);
    HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
  }
}

void usart2_init_as_console(uint32_t baud)
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
  stm32_uart2_ptr->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_TXINVERT_INIT;
  stm32_uart2_ptr->AdvancedInit.TxPinLevelInvert = UART_ADVFEATURE_TXINV_ENABLE;
  if (HAL_UART_Init(stm32_uart2_ptr) != HAL_OK)
  {
    fatal_error();
  }
}

// transmit to joycon via serial with flow control
void transmit_as_console(uint8_t *data, uint16_t size)
{
  for (int i = 0; i < size; ++i)
  {
    // first, look at flow control line, wait if TX not allowed
    while(HAL_GPIO_ReadPin(GPIOA, FC_NS_TX_EN_Pin) == GPIO_PIN_SET)
      ;
    // now send one byte
    HAL_UART_Transmit(stm32_uart2_ptr, data + i, 1, 1000);
  }
}

void init_as_console(void)
{
  // PA0, FC_JC_TX_EN, purple wire, Joycon will only send data to this board when this line is HIGH, should be output
  // PA1, FC_NS_TX_EN, green wire, this board will only send data to Joycon when this line is LOW, should be input

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = FC_JC_TX_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(FC_JC_TX_EN_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = FC_NS_TX_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(FC_NS_TX_EN_GPIO_Port, &GPIO_InitStruct);

  HAL_UART_MspDeInit(stm32_uart2_ptr);
  HAL_UART_MspInit(stm32_uart2_ptr);
  usart2_init_as_console(1000000);
  // enable flow control line, now Joycon can send commands
  JCTX_ENABLE();
}

void get_header_as_console()
{
  // every message exchange starts with a 4-byte header, so we'll get those first
  // receive 4 byte via interrupt, when complete the HAL_UART_RxCpltCallback()
  // in main.c is called
  HAL_UART_Receive_IT(stm32_uart2_ptr, switch_lb.buf, 4);
  JCTX_ENABLE();
  uart_status = UART_WAITING;
  // wait until received all 4 bytes
  while(uart_status != UART_RECEIVED)
  {
    // stop Joycon from further transmitting when receiving the last byte 
    // of the header so we have time to process it
    if(stm32_uart2_ptr->RxXferCount <= 1)
      JCTX_DISABLE();
  }
    
}

void get_payload_as_console()
{
  // now we have the header, we know how much more data to expect
  uint8_t* buf_start = switch_lb.buf + 4;
  // payload length is the last byte of the header, plus 1 byte of checksum
  uint8_t recv_size = switch_lb.buf[3] + 1;
  // now we just need to receive that amount
  HAL_UART_Receive_IT(stm32_uart2_ptr, buf_start, recv_size);
  JCTX_ENABLE();
  uart_status = UART_WAITING;
  while(uart_status != UART_RECEIVED)
    if(stm32_uart2_ptr->RxXferCount <= 1)
      JCTX_DISABLE();
}

void get_msg_as_console()
{
  // clear receive buffer
  linear_buf_reset(&switch_lb);
  get_header_as_console();
  get_payload_as_console();
}

void read_joycon(void)
{
  init_as_console();
  HAL_Delay(1);

  transmit_as_console(handshake_header, 4);
  transmit_as_console(connect_request, 12);
  get_msg_as_console();
  if(memcmp(switch_lb.buf, connect_request_response, 12) != 0)
    fatal_error();

  transmit_as_console(cmd2, 12);
  get_msg_as_console();
  if(memcmp(switch_lb.buf, cmd2_response, 20) != 0)
    fatal_error();

  transmit_as_console(cmd3, 20);
  get_msg_as_console();
  if(memcmp(switch_lb.buf, cmd3_response, 12) != 0)
    fatal_error();

  usart2_init_as_console(3125000);
  HAL_UART_AbortReceive(stm32_uart2_ptr);

  transmit_as_console(cmd4, 12);
  get_msg_as_console();
  if(memcmp(switch_lb.buf, cmd4_response, 12) != 0)
    fatal_error();

  transmit_as_console(cmd5, 12);
  get_msg_as_console();
  if(memcmp(switch_lb.buf, cmd5_response, 12) != 0)
    fatal_error();

  transmit_as_console(cmd6, 16);
  get_msg_as_console();
  if(memcmp(switch_lb.buf, cmd6_response, 12) != 0)
    fatal_error();

  // now handshake is complete, ask for updates periodically
  while(1)
  {
    transmit_as_console(update_request, 13);
    get_msg_as_console();
    for (int i = 0; i < 64; ++i)
      printf("0x%x ", switch_lb.buf[i]);
    printf("\n");
    HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
    HAL_Delay(100);
  }
}

