#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "helpers.h"
#include "shared.h"

#define INT32MAX (0x7FFFFFFF)

int32_t linear_buf_init(linear_buf *lb)
{
  if(lb == NULL)
    return 1;
  linear_buf_reset(lb);
  lb->last_recv = 0;
  lb->last_reset = 0;
  return 0;
}

int32_t linear_buf_reset(linear_buf *lb)
{
  if(lb == NULL)
    return 1;
  lb->curr_index = 0;
  memset(lb->buf, 0, LB_SIZE);
  lb->last_reset = HAL_GetTick();
  return 0;
}

int32_t linear_buf_add(linear_buf *lb, uint8_t c)
{
  if(lb == NULL)
    return 1;
  lb->buf[lb->curr_index] = c;
  if(lb->curr_index < LB_SIZE)
    lb->curr_index++;
  lb->last_recv = HAL_GetTick();
  return 0;
}

int32_t linear_buf_add_str(linear_buf *lb, uint8_t *s, uint32_t len)
{
  if(lb == NULL || s == NULL)
    return 1;
  for(uint32_t i = 0; i < len; i++)
    linear_buf_add(lb, s[i]);
  lb->buf[LB_SIZE-1] = 0;
  return 0;
}

int32_t linear_buf_line_available(linear_buf *lb)
{
  if(lb == NULL || lb->curr_index <= 0)
    return 0;
  if(lb->curr_index >= LB_SIZE)
  {
    linear_buf_reset(lb);
    return 0;
  }
  if(lb->buf[lb->curr_index - 1] == '\n')
    return 1;
  return 0;
}

int32_t linear_buf_idle(linear_buf *lb)
{
  if(lb == NULL || lb->curr_index <= 0)
    return 0;
  if(lb->curr_index >= LB_SIZE)
  {
    linear_buf_reset(lb);
    return 0;
  }
  if(HAL_GetTick() - lb->last_recv > 10)
    return 1;
  return 0;
}

void idwg_kick(void)
{
  if(HAL_GetTick() > next_iwdg_kick)
  {
    HAL_IWDG_Refresh(stm32_iwdg_ptr);
    next_iwdg_kick = HAL_GetTick() + 500;
  }
}

void enter_standby(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_Delay(1);
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN6);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN6);
  HAL_PWR_EnterSTANDBYMode();
}
