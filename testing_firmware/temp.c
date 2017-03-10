if(HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET)
    {
      get_packet();
      memcpy(frame1, switch_bf, SWITCH_BUF_SIZE);
      memset(switch_bf, 0, SWITCH_BUF_SIZE);

      get_packet();
      memcpy(frame2, switch_bf, SWITCH_BUF_SIZE);
      memset(switch_bf, 0, SWITCH_BUF_SIZE);

      get_packet();
      memcpy(frame3, switch_bf, SWITCH_BUF_SIZE);
      memset(switch_bf, 0, SWITCH_BUF_SIZE);

      get_packet();
      memcpy(frame4, switch_bf, SWITCH_BUF_SIZE);
      memset(switch_bf, 0, SWITCH_BUF_SIZE);

      print_packet(frame1);
      print_packet(frame2);
      print_packet(frame3);
      print_packet(frame4);

      HAL_Delay(1000);
    }

for (int i = 0; i < 4; ++i)
      {
        get_packet();
        memcpy(frame1, switch_bf, SWITCH_BUF_SIZE);
        // print_packet();
        memset(switch_bf, 0, SWITCH_BUF_SIZE);
      }

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  HAL_UART_Receive_IT(&huart1, switch_bf, 1);
  // HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
  // if(huart->Instance==USART1)
  // {
  //     linear_buf_add(&debug_lb, switch_bf[0]);
  //     HAL_UART_Receive_IT(&huart1, switch_bf, 1);
  // }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
  // if(huart->Instance==USART1)
  // {
  //     HAL_UART_Receive_IT(&huart1, switch_bf, 1);
  //     linear_buf_reset(&debug_lb);
  // }
}

idwg_kick();
    usb_data = my_usb_readline();
    if(usb_data != NULL)
      parse_cmd(usb_data); 

  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    idwg_kick();
    usb_data = my_usb_readline();
    if(usb_data != NULL)
      parse_cmd(usb_data); 
  }

if(HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET)
    {
      HAL_GPIO_WritePin(KEYPAD_COL1_GPIO_Port, KEYPAD_COL1_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_RESET);
      delay_us(1215);
      HAL_GPIO_WritePin(KEYPAD_COL1_GPIO_Port, KEYPAD_COL1_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_SET);
      HAL_Delay(1000);
    }

while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    if(HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET)
    {
      HAL_GPIO_WritePin(DUP_BUTTON_GPIO_Port, DUP_BUTTON_Pin, GPIO_PIN_SET);
      while(HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET)
      {
        HAL_GPIO_WritePin(KEYPAD_COL1_GPIO_Port, KEYPAD_COL1_Pin, GPIO_PIN_SET);
        delay_us(232);
        HAL_GPIO_WritePin(KEYPAD_COL1_GPIO_Port, KEYPAD_COL1_Pin, GPIO_PIN_RESET);
        delay_us(40);
      }
      HAL_GPIO_WritePin(DUP_BUTTON_GPIO_Port, DUP_BUTTON_Pin, GPIO_PIN_RESET);
      HAL_Delay(1000);
    }

![Alt text](http://i.imgur.com/9fGDsj6.jpg)

I simply removed the rumble motor, burned a hole on the back cover, and routed all the wires out through that and soldered on a header.
