#ifndef __MY_USB_H
#define __MY_USB_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f0xx_hal.h"
#include "helpers.h"

extern linear_buf usb_recv_buf;
extern int32_t is_port_open;
extern char* usb_data;

void my_usb_init(void);
void my_usb_putchar(uint8_t ch);
char* my_usb_readline(void);

#ifdef __cplusplus
}
#endif

#endif
