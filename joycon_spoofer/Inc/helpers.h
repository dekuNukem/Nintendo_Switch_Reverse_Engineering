#ifndef __HELPERS_H
#define __HELPERS_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f0xx_hal.h"

#define LB_SIZE 256

typedef struct
{
	int32_t last_recv;
	int32_t curr_index;
	int32_t msg_complete;
	uint8_t buf[LB_SIZE];
} linear_buf;

int32_t linear_buf_init(linear_buf *lb);
int32_t linear_buf_reset(linear_buf *lb);
int32_t linear_buf_idle(linear_buf *lb);
int32_t linear_buf_line_available(linear_buf *lb);
int32_t linear_buf_add(linear_buf *lb, uint8_t c);
int32_t linear_buf_add_str(linear_buf *lb, uint8_t *s, uint32_t len);
void idwg_kick(void);
void enter_standby(void);

#ifdef __cplusplus
}
#endif

#endif


