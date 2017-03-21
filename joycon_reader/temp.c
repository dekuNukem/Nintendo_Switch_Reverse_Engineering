 if not received.startswith("19 81 03 38"):
    continue
  for item in received.split('| '):
    print(item)
  received = received.replace('| ', '').split(' ')
  byte_list = [int(x, 16) for x in received if len(x) > 0]
  time.sleep(0.1)

  acc_1 = byte_2_int16(byte_list[26], byte_list[27])
  acc_2 = byte_2_int16(byte_list[30], byte_list[31])
  print(acc_1)
  print(acc_2)


    if(HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET)
    {
      HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_RESET);
      printf("%d\n", HAL_GetTick());
      read_joycon();
      HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_SET);
    }

    HAL_UART_Transmit(stm32_uart2_ptr, update_request, 13, 1000);

  for (int i = 0; i < 32; ++i)
    printf("0x%x ", switch_lb.buf[i]);
  printf("\ndone!\n");

  HAL_Delay(500);

void get_header()
{
  HAL_UART_Receive_IT(&huart2, switch_bb, SWITCH_BB_SIZE);
  NSTX_ENABLE();
  uart_status = UART_WAIT_4BHEADER;
  while(uart_status != UART_4BHEADER_RECEIVED)
  {
    if(huart2.RxXferCount <= 1)
      NSTX_DISABLE();
  }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  NSTX_DISABLE();
  HAL_UART_AbortReceive(huart);
  if(uart_status == UART_WAIT_4BHEADER)
  {
    linear_buf_add_str(&switch_lb, switch_bb, 4);
    uart_status = UART_4BHEADER_RECEIVED;
  }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  NSTX_DISABLE();
  linear_buf_add(&switch_lb, switch_bb[0]);
  HAL_UART_AbortReceive(&huart2);
  if(switch_lb.curr_index < 4)
    return;
  if(memcmp(switch_lb.buf, handshake_header, 4) == 0)
  {
    linear_buf_reset(&switch_lb);
    return;
  }
  if(switch_lb.curr_index >= switch_lb.buf[3] + 5)
    switch_lb.msg_complete = 1;
  return;
}

int32_t get_msg()
{
  NSTX_ENABLE();
  HAL_UART_Receive_IT(&huart2, switch_bb, 1);
  uint8_t* msg_start = switch_lb.buf;
  if(memcmp(switch_lb.buf, handshake_header, 4) == 0)
  {
    linear_buf_reset(&switch_lb);
    return 1;
  }
  if(switch_lb.curr_index < 4 || switch_lb.buf[3] < 7)
    return 2;
  if(switch_lb.curr_index < switch_lb.buf[3] + 4)
    return 3;
  return 0;
}
int32_t get_msg_blocking()
{
  int32_t now = HAL_GetTick();
  while(get_msg() == 0)
  {
    if(HAL_GetTick() - now > 1000)
    {
      for (int i = 0; i < 32; ++i)
        printf("0x%x ", switch_lb.buf[i]);
      printf("\ndone!\n");
      return 1;
    }
  }
  return 0;
}


void recv_1b()
{
  NSTX_ENABLE();
  HAL_UART_Receive_IT(&huart2, switch_bb, 1);
}
  for (int i = 0; i < 32; ++i)
    printf("0x%x ", switch_lb.buf[i]);
  printf("\ndone!\n");

checksum
{0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x10, 0x00, 0x00, 0x00, 0x00, 0x3D}
{0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x01, 0x00, 0x00, 0x00, 0x00, 0x24}
{0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x11, 0x00, 0x00, 0x00, 0x00, 0x0E}

{0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x10, 0x00, 0x00, 0x00, 0x00, 0xD6}
{0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x11, 0x00, 0x00, 0x0F, 0x00, 0x33}


uint8_t connect_request[16] = {0xA1, 0xA2, 0xA3, 0xA4, 0x19, 0x01, 0x03, 0x07, 0x00, 0xA5, 0x02, 0x01, 0x7E, 0x00, 0x00, 0x00};
uint8_t connect_request_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0xA5, 0x02, 0x02, 0x7D, 0x00, 0x00, 0x64};

uint8_t cmd2[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x01, 0x00, 0x00, 0x00, 0x00, 0x24};
uint8_t cmd2_response[20] = {0x19, 0x81, 0x03, 0x0F, 0x00, 0x94, 0x01, 0x08, 0x00, 0x00, 0xFA, 0xE8, 0x01, 0x31, 0x67, 0x9C, 0x8A, 0xBB, 0x7C, 0x00};

uint8_t cmd3[20] = {0x19, 0x01, 0x03, 0x0F, 0x00, 0x91, 0x20, 0x08, 0x00, 0x00, 0xBD, 0xB1, 0xC0, 0xC6, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t cmd3_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x20, 0x00, 0x00, 0x00, 0x00, 0xA8};

// first cmd after rate switch
uint8_t cmd4[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x11, 0x00, 0x00, 0x00, 0x00, 0x0E};
uint8_t cmd4_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x11, 0x00, 0x00, 0x0F, 0x00, 0x33};

uint8_t cmd5[13] = {0, 0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x10, 0x00, 0x00, 0x00, 0x00, 0x3D};
uint8_t cmd5_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x10, 0x00, 0x00, 0x00, 0x00, 0xD6};

uint8_t cmd6[16] = {0x19, 0x01, 0x03, 0x0B, 0x00, 0x91, 0x12, 0x04, 0x00, 0x00, 0x12, 0xA6, 0x0F, 0x00, 0x00, 0x00};
uint8_t cmd6_response[12] = {0x19, 0x81, 0x03, 0x07, 0x00, 0x94, 0x12, 0x00, 0x00, 0x00, 0x00, 0xB0};

uint8_t update_request[13] = {0x19, 0x01, 0x03, 0x08, 0x00, 0x92, 0x00, 0x01, 0x00, 0x00, 0x69, 0x2D, 0x1F};
// uint8_t update_request_reponse[61] = {0x19, 0x81, 0x03, 0x38, 0x00, 0x92, 0x00, 0x31, 0x00, 0x00, 0xD2, 0xD2, 0x30, 0xCA, 0x50, 0x00, 0x80, 0x00, 0x6F, 0x67, 0x84, 0x00, 0x00, 0x00, 0x90, 0x4D, 0x00, 0x2E, 0x00, 0x71, 0x10, 0xF7, 0xFF, 0xDD, 0xFF, 0xA5, 0x00, 0x9B, 0x00, 0x49, 0x00, 0x4A, 0x10, 0x36, 0x00, 0xD9, 0xFF, 0xA7, 0x00, 0x05, 0x01, 0x56, 0x00, 0xA3, 0x10, 0x3E, 0x00, 0xDE, 0xFF, 0xA7, 0x00};
uint8_t update_request_reponse[61] = 
{
  0x19, 0x81, 0x03, 0x38, 
  0x00, 0x92, 0x00, 0x31,
  0x00, 0x00, 0xB6, 0x2B,
  0x30, 0x20, 0x50, 0x00,
  0x80, 0x00, 0x6F, 0x87,
  0x84, 0x00, 0x00, 0x00,
  0xA0, 0x1D, 0x01, 0xD7,
  0xFF, 0x39, 0x10, 0x0A,
  0x00, 0xD7, 0xFF, 0xCF,
  0xFF, 0x1E, 0x01, 0xD7,
  0xFF, 0x38, 0x10, 0x0A,
  0x00, 0xD9, 0xFF, 0xD0,
  0xFF, 0x20, 0x01, 0xD3,
  0xFF, 0x38, 0x10, 0x0B,
  0x00, 0xD8, 0xFF, 0xCF,
  0xFF};

void get_packet(uint16_t size)
{
  HAL_UART_Receive_IT(&huart2, switch_bf, size);
  while(rx_complete == 0)
    ;
  HAL_UART_AbortReceive_IT(&huart2);
  rx_complete = 0;
}
void print_packet(void)
{
  for (int i = 1; i <= SWITCH_BUF_SIZE; ++i)
    printf("%02x ", switch_bf[i-1]);
  printf("\n");
  memset(switch_bf, 0, SWITCH_BUF_SIZE);
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

  get_packet(16);
  if(memcmp(switch_bf, connect_request, 16) != 0)
    fatal_error();
  transmit_with_cts(connect_request_response, 12);

  get_packet(12);
  if(memcmp(switch_bf, cmd2, 12) != 0)
    fatal_error();
  transmit_with_cts(cmd2_response, 20);

  // Here we get the insertion animation
  // so I guess cmd2_response is where the color of the joycon is

  get_packet(20);
  if(memcmp(switch_bf, cmd3, 20) != 0)
    fatal_error();
  transmit_with_cts(cmd3_response, 12);

  usart2_init_3125000();
  memset(switch_bf, 0, SWITCH_BUF_SIZE);
  HAL_Delay(2);
  transmit_with_cts(cmd4_response, 12);
  HAL_UART_AbortReceive_IT(&huart2);

  get_packet(13);
  if(memcmp(switch_bf, cmd5, 13) != 0)
    fatal_error();
  transmit_with_cts(cmd5_response, 12);

  get_packet(12);
  if(memcmp(switch_bf, cmd6, 12) != 0)
    fatal_error();
  transmit_with_cts(cmd6_response, 12);

  while(1)
  {
    transmit_with_cts(update_request_reponse, 61);
    HAL_Delay(15);
  }
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
  get_packet(16);
  if(memcmp(switch_bf, connect_request, 16) != 0)
    fatal_error();

  transmit_with_cts(connect_request_response, 12);
  get_packet(12);
  if(memcmp(switch_bf, cmd2, 12) != 0)
    fatal_error();

  transmit_with_cts(cmd2_response, 20);
  get_packet(20);
  if(memcmp(switch_bf, cmd3, 20) != 0)
    fatal_error();

  transmit_with_cts(cmd3_response, 12);

  usart2_init_3125000();
  HAL_Delay(5);
  while(HAL_GPIO_ReadPin(GPIOA, UART2_RTS_H_Pin) == GPIO_PIN_RESET)
    ;
  memset(switch_bf, 0, SWITCH_BUF_SIZE);
  get_packet(13);
  if(memcmp(switch_bf, cmd4, 13) != 0)
    fatal_error();

  transmit_with_cts(cmd4_response, 12);
  get_packet(12);
  if(memcmp(switch_bf, cmd5, 12) != 0)
    fatal_error();

  transmit_with_cts(cmd5_response, 12);
  get_packet(12);
  if(memcmp(switch_bf, cmd6, 12) != 0)
    fatal_error();

  transmit_with_cts(cmd6_response, 12);

  while(1)
  {
    get_packet(13);
    if(memcmp(switch_bf, update_request, 13) != 0)
      fatal_error();
    transmit_with_cts(update_request_reponse, 61);
  }
}

  usart2_init_3125000();
  HAL_Delay(5);
  while(HAL_GPIO_ReadPin(GPIOA, UART2_RTS_H_Pin) == GPIO_PIN_RESET)
    ;
  memset(switch_bf, 0, SWITCH_BUF_SIZE);
  get_packet(12);



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
  get_packet(16);
  if(memcmp(switch_bf, connect_request, 16) == 0)
  {
    HAL_UART_Transmit(&huart2, connect_request_response, 12, 1000);
    while(1);
  }
  print_packet();
}

void handshake()
{
  uint8_t start[4] = {0xA1, 0xA2, 0xA3, 0xA4};
  uint8_t cmd1[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0xA5, 0x02, 0x01, 0x7E, 0x00, 0x00, 0x00};
  uint8_t cmd2[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x01, 0x00, 0x00, 0x00, 0x00, 0x24};
  uint8_t cmd3[20] = {0x19, 0x01, 0x03, 0x0F, 0x00, 0x91, 0x20, 0x08, 0x00, 0x00, 0xBD, 0xB1, 0xC0, 0xC6, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x00};

  uint8_t cmd4[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x11, 0x00, 0x00, 0x00, 0x00, 0x0E};
  uint8_t cmd5[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x10, 0x00, 0x00, 0x00, 0x00, 0x3D};
  uint8_t cmd6[16] = {0x19, 0x01, 0x03, 0x0B, 0x00, 0x91, 0x12, 0x04, 0x00, 0x00, 0x12, 0xA6, 0x0F, 0x00, 0x00, 0x00};

  uint8_t update_request[13] = {0x19, 0x01, 0x03, 0x08, 0x00, 0x92, 0x00, 0x01, 0x00, 0x00, 0x69, 0x2D, 0x1F};
  
  RTS_ON();
  HAL_Delay(100);
  HAL_UART_Transmit(&huart2, start, 4, 1000);
  delay_us(44);
  HAL_UART_Transmit(&huart2, cmd1, 12, 1000);
  get_packet(12);
  HAL_UART_Transmit(&huart2, cmd2, 12, 1000);
  get_packet(20);

  HAL_UART_Transmit(&huart2, cmd4, 12, 1000);
  get_packet(12);
  delay_us(100);
  HAL_UART_Transmit(&huart2, cmd5, 12, 1000);
  get_packet(12);
  delay_us(100);
  HAL_UART_Transmit(&huart2, cmd6, 16, 1000);
  get_packet(12);
  delay_us(100);

  HAL_UART_Transmit(&huart2, update_request, 13, 1000);
  for (int i = 0; i < 10; ++i)
  {
    HAL_UART_Transmit(&huart2, update_request, 13, 1000);
    HAL_Delay(15);
  }
  HAL_Delay(1000);
  RTS_OFF();
}


void handshake()
{
  uint8_t start[4] = {0xA1, 0xA2, 0xA3, 0xA4};
  uint8_t cmd1[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0xA5, 0x02, 0x01, 0x7E, 0x00, 0x00, 0x00};
  uint8_t cmd2[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x01, 0x00, 0x00, 0x00, 0x00, 0x24};
  uint8_t cmd3[20] = {0x19, 0x01, 0x03, 0x0F, 0x00, 0x91, 0x20, 0x08, 0x00, 0x00, 0xBD, 0xB1, 0xC0, 0xC6, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x00};

  uint8_t cmd4[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x11, 0x00, 0x00, 0x00, 0x00, 0x0E};
  uint8_t cmd5[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x10, 0x00, 0x00, 0x00, 0x00, 0x3D};
  uint8_t cmd6[16] = {0x19, 0x01, 0x03, 0x0B, 0x00, 0x91, 0x12, 0x04, 0x00, 0x00, 0x12, 0xA6, 0x0F, 0x00, 0x00, 0x00};

  uint8_t update_request[13] = {0x19, 0x01, 0x03, 0x08, 0x00, 0x92, 0x00, 0x01, 0x00, 0x00, 0x69, 0x2D, 0x1F};
  
  RTS_ON();
  HAL_Delay(100);
  HAL_UART_Transmit(&huart2, start, 4, 1000);
  delay_us(44);
  HAL_UART_Transmit(&huart2, cmd1, 12, 1000);
  get_packet(12);
  HAL_UART_Transmit(&huart2, cmd2, 12, 1000);
  get_packet(20);

  HAL_UART_Transmit(&huart2, cmd3, 20, 1000);
  get_packet(12);

  usart2_init_3125000();
  while(HAL_GPIO_ReadPin(JOYCON_CTS_GPIO_Port, JOYCON_CTS_Pin) == GPIO_PIN_SET)
    ;

  HAL_UART_Transmit(&huart2, cmd4, 12, 1000);
  get_packet(12);
  delay_us(100);
  HAL_UART_Transmit(&huart2, cmd5, 12, 1000);
  get_packet(12);
  delay_us(100);
  HAL_UART_Transmit(&huart2, cmd6, 16, 1000);
  get_packet(12);
  delay_us(100);

  HAL_UART_Transmit(&huart2, update_request, 13, 1000);
  for (int i = 0; i < 10; ++i)
  {
    HAL_UART_Transmit(&huart2, update_request, 13, 1000);
    HAL_Delay(15);
  }
  HAL_Delay(1000);
  RTS_OFF();
}
void handshake()
{
  uint8_t start[4] = {0xA1, 0xA2, 0xA3, 0xA4};
  uint8_t cmd1[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0xA5, 0x02, 0x01, 0x7E, 0x00, 0x00, 0x00};
  uint8_t cmd2[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x01, 0x00, 0x00, 0x00, 0x00, 0x24};
  uint8_t cmd3[20] = {0x19, 0x01, 0x03, 0x0F, 0x00, 0x91, 0x20, 0x08, 0x00, 0x00, 0xBD, 0xB1, 0xC0, 0xC6, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x00};

  uint8_t cmd4[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x11, 0x00, 0x00, 0x00, 0x00, 0x0E};
  uint8_t cmd5[12] = {0x19, 0x01, 0x03, 0x07, 0x00, 0x91, 0x10, 0x00, 0x00, 0x00, 0x00, 0x3D};
  uint8_t cmd6[16] = {0x19, 0x01, 0x03, 0x0B, 0x00, 0x91, 0x12, 0x04, 0x00, 0x00, 0x12, 0xA6, 0x0F, 0x00, 0x00, 0x00};

  huart2.Init.BaudRate = 1000000;
  if(HAL_UART_Init(&huart2) != HAL_OK)
    Error_Handler();
  
  RTS_ON();
  HAL_Delay(100);
  HAL_UART_Transmit(&huart2, start, 4, 1000);
  delay_us(44);
  HAL_UART_Transmit(&huart2, cmd1, 12, 1000);
  get_packet(12);
  // print_packet();

  HAL_UART_Transmit(&huart2, cmd2, 12, 1000);
  get_packet(20);
  // print_packet();

  HAL_UART_Transmit(&huart2, cmd3, 20, 1000);
  get_packet(12);
  // print_packet();
  delay_us(200);
  RTS_OFF();
  delay_us(300);


  HAL_UART_MspDeInit(&huart2);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  RTS_ON();
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
  delay_us(110);
  RTS_OFF();
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
  RTS_ON();

  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);
  HAL_UART_MspInit(&huart2);
  usart2_init_3125000();

  while(HAL_GPIO_ReadPin(JOYCON_CTS_GPIO_Port, JOYCON_CTS_Pin) == GPIO_PIN_SET)
    ;

  HAL_UART_Transmit(&huart2, cmd4, 12, 1000);
  delay_us(1000);
  HAL_UART_Transmit(&huart2, cmd5, 12, 1000);
  delay_us(1000);
  HAL_UART_Transmit(&huart2, cmd6, 16, 1000);
  delay_us(1000);


  HAL_Delay(100);
  RTS_OFF();
}


uint8_t recv_buf[4];
HAL_UART_Receive(&huart2, recv_buf, 4, 2);
 switch(result)
  {
    case ARG_PARSE_SUCCESS:
    button_ctrl(GPIO_PIN_RESET);
    puts("bh OK");
    break;

    case ARG_PARSE_ERROR_INVALID_CMD:
    puts("bh ERROR: invalid command");
    break;

    case ARG_PARSE_ERROR_NOT_AVAILABLE:
    puts("bh ERROR: button not available");
    break;

    default:
    puts("bh ERROR: unknown");
  }
usb_data = my_usb_readline();
    if(usb_data != NULL)
    {
      printf("I said \"%s\"\n", usb_data);
      handshake();
    }

  int32_t get_arg(char* argstr, uint32_t index)
{
  if(argstr == NULL)
    return INT32MAX;

  char* str_end = argstr + strlen(argstr);
  char* curr = argstr;
  int32_t count = 0;

  while(curr < str_end && *curr != ' ')
    curr++;
  curr++;
  if(curr >= str_end)
    return INT32MAX - 1;
  if(index == 0)
    return atoi(curr);

  while(curr < str_end)
  {
    if(*curr == ' ')
      count++;
    if(count == index)
      break;
    curr++;
  }
  curr++;
  if(curr >= str_end)
    return INT32MAX - 2;
  return atoi(curr);
}


  // built in dac starts in high-z mode
  HAL_DAC_DeInit(stm32_dac_ptr);
  printf("%d, %d\n", x_12b, y_12b);

  else if(strncmp(cmd, "sd", 2) == 0)
  {
    HAL_DAC_DeInit(stm32_dac_ptr);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = STM32_DAC_X_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(STM32_DAC_X_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = STM32_DAC_Y_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(STM32_DAC_Y_GPIO_Port, &GPIO_InitStruct);

    puts("sd OK");
  }
void button_ctrl(int32_t action)
{
  for(int i = 0; i < ARG_QUEUE_SIZE; ++i)
  {
    if(gpio_port_queue[i] != NULL)
    {
      printf("acting on %d...\n", i);
      if(action == ACTION_HOLD)
        HAL_GPIO_WritePin(gpio_port_queue[i], gpio_pin_queue[i], GPIO_PIN_RESET);
      else
        HAL_GPIO_WritePin(gpio_port_queue[i], gpio_pin_queue[i], GPIO_PIN_SET);
    }
  }
}

int32_t endswith(char* source, char* query)
{
  return strncmp(source, query, 4) == 0 && strlen(source) == strlen(query);
}

    for (int i = 0; i < ARG_QUEUE_SIZE; ++i)
      printf("%d: %d\n", i, gpio_pin_queue[i]);
void release_all_buttons(void)
{
  puts("bra ok");
}


  char* my_usb_readline(void)
{
  char* ret = NULL;
  if(usb_recv_buf.curr_index > 0)
  {
    if(usb_recv_buf.buf[usb_recv_buf.curr_index - 1] == '\n')
    {
      for (int i = 0; i < usb_recv_buf.curr_index; ++i)
        if(usb_recv_buf.buf[i] == '\n' || usb_recv_buf.buf[i] == '\r')
          usb_recv_buf.buf[i] = 0;
      memset(usb_line_buf, 0, LB_SIZE);
      strcpy(usb_line_buf, usb_recv_buf.buf);
      ret = usb_line_buf;
      linear_buf_reset(&usb_recv_buf);
    }
    else if(HAL_GetTick() - usb_recv_buf.last_recv > 250)
      linear_buf_reset(&usb_recv_buf);
  }
  return ret;
}




  if(strncmp(cmd, "test\n", 5) == 0)
    printf("im alive!\n");
  for (int i = 0; i < strlen(cmd); ++i)
    printf("%c %d\n", cmd[i], cmd[i]);
void parse_cmd(char* cmd)
{
  if(cmd == NULL)
    return;

  if(strncmp(cmd, "test\n", 5) == 0)
    puts("test OK");
  else if(strncmp(cmd, "bh\n", 3) == 0)
  {
    puts("bh OK");
    HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_RESET);
  }
  else if(strncmp(cmd, "br\n", 3) == 0)
  {
    puts("br OK");
    HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_SET);
  }
  else
    puts("ERROR");
}

        usb_line_buf[strlen(usb_line_buf)] = '\n';


  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    usb_data = my_usb_readline();
    if(usb_data != NULL)
    {
      // printf("usb_lb: %s\n", usb_data);
      // stm_dac_test();
      serial_test_flag = (serial_test_flag + 1) % 2;
      printf("serial_test: %d\n", serial_test_flag);
    }
    if(serial_test_flag)
      my_usb_putchar('k');
  }


HAL_DAC_DeInit(stm32_dac_ptr);

// max dac
void test(void)
{
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

// bulit in dac
void test(void)
{
  printf("run started\n");
  uint16_t count = 0;
  while(1)
  {
    // HAL_DAC_SetValue(stm32_dac_ptr, DAC_CHANNEL_1, DAC_ALIGN_8B_R, count);
    // HAL_DACEx_DualSetValue(stm32_dac_ptr, DAC_ALIGN_8B_R, count, 255 - count);
    HAL_DACEx_DualSetValue(stm32_dac_ptr, DAC_ALIGN_12B_R, count, 65535 - count);
    count++;
  }
}


if(linear_buf_line_available(&debug_lb))
		{
		  printf("debug_lb: %s\n", debug_lb.buf);

		  if(strstr(debug_lb.buf, "s ") != NULL)
		  {
		  	uint8_t to_send = get_arg(debug_lb.buf, 0);
		  	uint8_t received = 0;
		  	printf("sending %d...\n", to_send);
		  	spi_cs_low();
		  	HAL_SPI_TransmitReceive(max572x_spi_ptr, &to_send, &received, 1, 500);
		  	spi_cs_high();
		  	printf("received: %d\n", received);
		  }
		  linear_buf_reset(&debug_lb);
		}


  SPI2_CS_GPIO_Port->ODR |= 0x1000; // CS high
  SPI2_CS_GPIO_Port->ODR &= 0xefff; // CS low


void max572x_POWER(uint8_t dac_multi_sel, uint8_t power_mode)
{
  uint8_t power = (uint8_t)((power_mode << 6) & 0xc0);
  printf("power: 0x%X\n", power);
  // uint8_t to_send[3] = {0x40, dac_multi_sel, (power_mode << 6) & 0xc0};
}

  printf("sending 0x%X 0x%X 0x%X...\n", to_send[0], to_send[1], to_send[2]);
void max572x_POWER(uint8_t dac_multi_sel, uint8_t power_mode)
{
  uint8_t to_send[3] = {0x40, dac_multi_sel, (uint8_t)((power_mode << 6) & 0xc0)};
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, to_send, 3, 100);
  spi_cs_high();
}

void read_status()
{
  uint8_t spi_sb[3] = {0xe8, 0xaa, 0xaa};
  uint8_t spi_rb[3] = {0, 0, 0};
  printf("sending 0x%X 0x%X 0x%X...\n", spi_sb[0], spi_sb[1], spi_sb[2]);
  spi_cs_low();
  HAL_SPI_TransmitReceive(max572x_spi_ptr, spi_sb, spi_rb, 3, 500);
  spi_cs_high();
  printf("spi_rb: 0x%X 0x%X 0x%X\n", spi_rb[0], spi_rb[1], spi_rb[2]);
}

void read_status()
{
  uint8_t spi_sb[3] = {0xe8, 0xaa, 0xaa};
  uint8_t spi_rb[3] = {0, 0, 0};
  printf("sending: ");
  print_3b(spi_sb);
  spi_cs_low();
  HAL_SPI_TransmitReceive(max572x_spi_ptr, spi_sb, spi_rb, 3, 500);
  spi_cs_high();
  printf("received: ");
  print_3b(spi_rb);
}

void max5725_CODEn(uint8_t dac_sel, uint16_t data)
{
  uint8_t spi_sb[3] = {0x80, 0, 0};
  dac_sel &= 0xf;
  spi_sb[0] |= dac_sel;
  data &= 0xfff;
  spi_sb[1] |= data >> 4;
  data &= 0xf;
  spi_sb[2] |= (data << 4);
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
  // printf("max5725_CODEn: "); print_3b(spi_sb);
}

void max5723_CODEn(uint8_t dac_sel, uint8_t data)
{
  uint8_t spi_sb[3] = {0x80, 0, 0};
  dac_sel &= 0xf;
  spi_sb[0] |= dac_sel;
  spi_sb[1] = data;
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
  // printf("max5723_CODEn: "); print_3b(spi_sb);
}

void pack_dac_val(uint8_t part, uint16_t value, uint8_t* dh, uint8_t* dl)
{
  if(part == MAX5723)
    *dh = (value & 0xff);
  else if(part == MAX5724)
  {
    value &= 0x3ff;
    *dh |= value >> 2;
    value &= 0x3;
    *dl |= (value << 6);
  }
  else
  {
    value &= 0xfff;
    *dh |= value >> 4;
    value &= 0xf;
    *dl |= (value << 4);
  }
}


void max5723_CODEn(uint8_t dac_sel, uint8_t data)
{
  uint8_t spi_sb[3] = {0x80, 0, 0};
  dac_sel &= 0xf;
  spi_sb[0] |= dac_sel;
  spi_sb[1] = data;
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
  // printf("max5723_CODEn: "); print_3b(spi_sb);
}

void max5724_CODEn(uint8_t dac_sel, uint16_t data)
{
  uint8_t spi_sb[3] = {0x80, 0, 0};
  dac_sel &= 0xf;
  spi_sb[0] |= dac_sel;
  data &= 0x3ff;
  spi_sb[1] |= data >> 2;
  data &= 0x3;
  spi_sb[2] |= (data << 6);
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
  // printf("max5724_CODEn: "); print_3b(spi_sb);
}

void max5725_CODEn(uint8_t dac_sel, uint16_t data)
{
  uint8_t spi_sb[3] = {0x80, 0, 0};
  dac_sel &= 0xf;
  spi_sb[0] |= dac_sel;
  data &= 0xfff;
  spi_sb[1] |= data >> 4;
  data &= 0xf;
  spi_sb[2] |= (data << 4);
  spi_cs_low();
  HAL_SPI_Transmit(max572x_spi_ptr, spi_sb, 3, 100);
  spi_cs_high();
  // printf("max5725_CODEn: "); print_3b(spi_sb);
}

// printf("max572x_SW_RESET\n");
// printf("max572x_POWER: "); print_3b(spi_sb);
// printf("max572x_CONFIG: "); print_3b(spi_sb);
// printf("max572x_WDOG: "); print_3b(spi_sb);
// printf("max572x_REF: "); print_3b(spi_sb);
// printf("max572x_CODEn: "); print_3b(spi_sb);
// printf("max572x_LOADn: "); print_3b(spi_sb);

void print_3b(uint8_t buf[3])
{
  printf("0x%x 0x%x 0x%x\n", buf[0], buf[1], buf[2]);
}

#define MAX5723 0
#define MAX5724 1
#define MAX5725 2

int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (unsigned char *)&ch, 1, 100);
    return ch;
}
while (1)
  {
    HAL_UART_Receive_IT(&huart1, debug_byte_buf, 1);
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    if(linear_buf_line_available(&debug_lb))
    {
      printf("debug_lb: %s\n", debug_lb.buf);
      test();
      linear_buf_reset(&debug_lb);
    }
  }

  while(1)
  {
    usb_data = my_usb_readline();
    if(usb_data != NULL)
    {
      printf("I received: %s\n", usb_data);
    }
  }

uint8_t debug_byte_buf[1];
linear_buf debug_lb;
extern UART_HandleTypeDef huart1;
#define debug_uart_ptr (&huart1)
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance==USART1)
  {
      linear_buf_add(&debug_lb, debug_byte_buf[0]);
      HAL_UART_Receive_IT(&huart1, debug_byte_buf, 1);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance==USART1)
  {
      HAL_UART_Receive_IT(&huart1, debug_byte_buf, 1);
      linear_buf_reset(&debug_lb);
  }
}
      HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);