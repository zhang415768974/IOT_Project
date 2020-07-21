#ifndef __TIMER_H__
#define __TIMER_H__

#include <stm32f10x.h>


void tim2_init(u16 arr, u16 psc, u8 priority);
void tim3_init(u16 arr, u16 psc, u8 priority);
void tim4_init(u16 arr, u16 psc, u8 priority);

#endif
