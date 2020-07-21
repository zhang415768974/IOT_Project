#ifndef __SYS_H__
#define __SYS_H__

#include <stm32f10x.h>

void iwdg_init(void);
void delay_ms(u16 ms);
void led_init(void);

void led_double(u16 interval);

#endif
