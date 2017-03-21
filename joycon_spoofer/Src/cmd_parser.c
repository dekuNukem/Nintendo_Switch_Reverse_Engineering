#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "helpers.h"
#include "shared.h"
#include "cmd_parser.h"

#define ARG_PARSE_SUCCESS 0
#define ARG_PARSE_ERROR_INVALID_CMD 126
#define ARG_QUEUE_SIZE 16
#define SWITCH_PRO_CONTROLLER_BUTTON_COUNT 19

uint16_t gpio_pin_queue[ARG_QUEUE_SIZE];
GPIO_TypeDef* gpio_port_queue[ARG_QUEUE_SIZE];

uint16_t gpio_pin_map[SWITCH_PRO_CONTROLLER_BUTTON_COUNT] = 
{
  DEBUG_LED_Pin,  // 0 dpad up
  DEBUG_LED_Pin,  // 1 dpad down
  DEBUG_LED_Pin,  // 2 dpad left
  DEBUG_LED_Pin,  // 3 dpad right
  DEBUG_LED_Pin,  // 4 L
  DEBUG_LED_Pin,  // 5 ZL
  DEBUG_LED_Pin,  // 6 minus
  DEBUG_LED_Pin,  // 7 capture
  DEBUG_LED_Pin,  // 8 LSB
  DEBUG_LED_Pin,  // 9 A
  DEBUG_LED_Pin,  // 10 B
  DEBUG_LED_Pin,  // 11 X
  DEBUG_LED_Pin,  // 12 Y
  DEBUG_LED_Pin,  // 13 R
  DEBUG_LED_Pin,  // 14 ZR
  DEBUG_LED_Pin,  // 15 plus
  DEBUG_LED_Pin,  // 16 home
  DEBUG_LED_Pin,  // 17 RSB
  DEBUG_LED_Pin,  // 18 SYNC
};

GPIO_TypeDef* gpio_port_map[SWITCH_PRO_CONTROLLER_BUTTON_COUNT] = 
{
  DEBUG_LED_GPIO_Port,  // 0 dpad up
  DEBUG_LED_GPIO_Port,  // 1 dpad down
  DEBUG_LED_GPIO_Port,  // 2 dpad left
  DEBUG_LED_GPIO_Port,  // 3 dpad right
  DEBUG_LED_GPIO_Port,  // 4 L
  DEBUG_LED_GPIO_Port,  // 5 ZL
  DEBUG_LED_GPIO_Port,  // 6 minus
  DEBUG_LED_GPIO_Port,  // 7 capture
  DEBUG_LED_GPIO_Port,  // 8 LSB
  DEBUG_LED_GPIO_Port,  // 9 A
  DEBUG_LED_GPIO_Port,  // 10 B
  DEBUG_LED_GPIO_Port,  // 11 X
  DEBUG_LED_GPIO_Port,  // 12 Y
  DEBUG_LED_GPIO_Port,  // 13 R
  DEBUG_LED_GPIO_Port,  // 14 ZR
  DEBUG_LED_GPIO_Port,  // 15 plus
  DEBUG_LED_GPIO_Port,  // 16 home
  DEBUG_LED_GPIO_Port,  // 17 RSB
  DEBUG_LED_GPIO_Port,  // 18 SYNC
};

char* goto_next_arg(char* buf)
{
  char* curr = buf;
  if(curr == NULL)
    return NULL;
  char* buf_end = curr + strlen(curr);
  if(curr >= buf_end)
    return NULL;
  while(curr < buf_end && *curr != ' ')
      curr++;
  while(curr < buf_end && *curr == ' ')
      curr++;
  if(curr >= buf_end)
    return NULL;
  return curr;
}

int32_t arg_to_button_index(char* cmd)
{
  if(strncmp(cmd, "du", 2) == 0)
    return 0;
  else if(strncmp(cmd, "dd", 2) == 0)
    return 1;
  else if(strncmp(cmd, "dl", 2) == 0)
    return 2;
  else if(strncmp(cmd, "dr", 2) == 0)
    return 3;
  else if(strncmp(cmd, "ls", 2) == 0)
    return 4;
  else if(strncmp(cmd, "zl", 2) == 0)
    return 5;
  else if(strncmp(cmd, "-", 1) == 0)
    return 6;
  else if(strncmp(cmd, "cap", 3) == 0)
    return 7;
  else if(strncmp(cmd, "sbl", 3) == 0)
    return 8;
  else if(strncmp(cmd, "a", 1) == 0)
    return 9;
  else if(strncmp(cmd, "b", 1) == 0)
    return 10;
  else if(strncmp(cmd, "x", 1) == 0)
    return 11;
  else if(strncmp(cmd, "y", 1) == 0)
    return 12;
  else if(strncmp(cmd, "rs", 2) == 0)
    return 13;
  else if(strncmp(cmd, "zr", 2) == 0)
    return 14;
  else if(strncmp(cmd, "+", 1) == 0)
    return 15;
  else if(strncmp(cmd, "h", 1) == 0)
    return 16;
  else if(strncmp(cmd, "sbr", 3) == 0)
    return 17;
  else if(strncmp(cmd, "sync", 4) == 0)
    return 18;
  return ARG_PARSE_ERROR_INVALID_CMD;
}

int32_t process_multiarg(char* args)
{
  char* arg_ptr = args;
  int32_t count = 0;
  int32_t result;
  memset(gpio_port_queue, 0, ARG_QUEUE_SIZE * sizeof(GPIO_TypeDef*));
  memset(gpio_pin_queue, 0, ARG_QUEUE_SIZE * sizeof(uint16_t));
  while(1)
  {
    if(arg_ptr == NULL || count >= ARG_QUEUE_SIZE)
      break;
    result = arg_to_button_index(arg_ptr);
    if(result >= 0 && result < SWITCH_PRO_CONTROLLER_BUTTON_COUNT)
    {
      count++;
      gpio_port_queue[count] = gpio_port_map[result];
      gpio_pin_queue[count] = gpio_pin_map[result];
    }
    else
      return result;
    arg_ptr = goto_next_arg(arg_ptr);
  }
  if(count <= 0)
    return ARG_PARSE_ERROR_INVALID_CMD;
  return ARG_PARSE_SUCCESS;
}

void button_ctrl(GPIO_PinState action)
{
  for(int i = 0; i < ARG_QUEUE_SIZE; ++i)
    if(gpio_port_queue[i] != NULL)
      HAL_GPIO_WritePin(gpio_port_queue[i], gpio_pin_queue[i], action);
}

int32_t button_hold(char* cmd)
{
  char* arg_start = goto_next_arg(cmd);
  int32_t result = process_multiarg(arg_start);
  if(result == ARG_PARSE_SUCCESS)
    button_ctrl(GPIO_PIN_RESET);
  return result;
}

int32_t button_release(char* cmd)
{
  char* arg_start = goto_next_arg(cmd);
  int32_t result = process_multiarg(arg_start);
  if(result == ARG_PARSE_SUCCESS)
    button_ctrl(GPIO_PIN_SET);
  return result;
}

int32_t stick_hold(char* cmd)
{
  char* x_ptr = goto_next_arg(cmd);
  char* y_ptr = goto_next_arg(x_ptr);
  uint32_t x_8b = atoi(x_ptr);
  uint32_t y_8b = atoi(y_ptr);
  uint32_t x_12b = (uint32_t)((float)x_8b * 8.76);
  uint32_t y_12b = (uint32_t)((float)y_8b * 8.76);
  if(x_ptr == NULL || y_ptr == NULL || x_8b > 255 || y_8b > 255)
    return ARG_PARSE_ERROR_INVALID_CMD;
  if(stm32_dac_ptr->State == HAL_DAC_STATE_RESET)
    stm32_dac_init();
  HAL_DACEx_DualSetValue(stm32_dac_ptr, DAC_ALIGN_12B_R, x_12b, y_12b);
  return ARG_PARSE_SUCCESS;
}

void stick_release(void)
{
  if(stm32_dac_ptr->State == HAL_DAC_STATE_RESET)
    stm32_dac_init();
  HAL_DACEx_DualSetValue(stm32_dac_ptr, DAC_ALIGN_12B_R, 1117, 1117);
}

void stick_disengage(void)
{
  if(stm32_dac_ptr->State != HAL_DAC_STATE_RESET)
  {
    HAL_DAC_Stop(stm32_dac_ptr, DAC_CHANNEL_1);
    HAL_DAC_Stop(stm32_dac_ptr, DAC_CHANNEL_2);
    HAL_DAC_DeInit(stm32_dac_ptr);
  }
}

void release_all_button(void)
{
  for(int i = 0; i < SWITCH_PRO_CONTROLLER_BUTTON_COUNT; ++i)
      HAL_GPIO_WritePin(gpio_port_map[i], gpio_pin_map[i], GPIO_PIN_SET);
}

void parse_cmd(char* cmd)
{
  int32_t result;
  if(strcmp(cmd, "test") == 0)
    puts("test OK");
  else if(strcmp(cmd, "whoami") == 0)
    puts("BOARD_TYPE_NDAC_PRO_CTRLER");
  // button hold, multiple args allowed
  else if(strncmp(cmd, "bh ", 3) == 0)
  {
    result = button_hold(cmd);
    switch(result)
    {
      case ARG_PARSE_SUCCESS:
      puts("bh OK");
      break;
      case ARG_PARSE_ERROR_INVALID_CMD:
      puts("bh ERROR: invalid command");
      break;
      default:
      puts("bh ERROR: unknown");
    }
  }
  // button release, multiple args allowed
  else if(strncmp(cmd, "br ", 3) == 0)
  {
    result = button_release(cmd);
    switch(result)
    {
      case ARG_PARSE_SUCCESS:
      puts("br OK");
      break;
      case ARG_PARSE_ERROR_INVALID_CMD:
      puts("br ERROR: invalid command");
      break;
      default:
      puts("br ERROR: unknown");
    }
  }
  // button release all
  else if(strcmp(cmd, "bra") == 0)
  {
    release_all_button();
    puts("bra OK");
  }
  
  // stick hold, sh x y, x and y between 0 and 255 inclusive
  else if(strncmp(cmd, "sh ", 3) == 0)
  {
    result = stick_hold(cmd);
    switch(result)
    {
      case ARG_PARSE_SUCCESS:
      puts("sh OK");
      break;
      case ARG_PARSE_ERROR_INVALID_CMD:
      puts("sh ERROR: invalid command");
      break;
      default:
      puts("sh ERROR: unknown");
    }
  }
  // stick release, to netural position 
  else if(strcmp(cmd, "sr") == 0)
  {
    stick_release();
    puts("sr OK");
  }
  // stick disengage, gives control back to user
  else if(strcmp(cmd, "sd") == 0)
  {
    stick_disengage();
    puts("sd OK");
  }
  // release all buttons and sticks
  else if(strcmp(cmd, "reset") == 0)
  {
    release_all_button();
    stick_disengage();
    puts("reset OK");
  }
  else
  {
    puts("ERROR unknown command");
  }
}
