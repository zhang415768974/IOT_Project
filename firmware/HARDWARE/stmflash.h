#ifndef __STMFLASH_H__
#define __STMFLASH_H__

#include <stm32f10x.h>

void flash_write(u32 addr, const char* buf, u16 size);

#endif
