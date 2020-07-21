#include "stmflash.h"

#define FLASH_KEY1	0x45670123
#define FLASH_KEY2	0xCDEF89AB

void flash_write(u32 addr, const char* buf, u16 size) {
		u32 secpos;
	u16 i;
	if (addr < FLASH_BASE || addr > FLASH_BASE + 0x10000 - size) {
		return;
	}
	secpos = FLASH_BASE + ((addr - FLASH_BASE) / 1024) * 1024;
	// 写入解锁序列
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
	// 页擦除
	while ((FLASH->SR & FLASH_SR_BSY) != RESET);
	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR = secpos;
	FLASH->CR |= FLASH_CR_STRT;
	// 等待操作完成
	while ((FLASH->SR & FLASH_SR_BSY) != RESET);
	FLASH->CR &= ~FLASH_CR_PER;
	// 写扇区
	FLASH->CR |= FLASH_CR_PG;
	for (i = 0; i <= size / 2; ++i) {
		*(u16*)secpos = *((u16*)buf + i);
		secpos += 2;
		while ((FLASH->SR & FLASH_SR_BSY) != RESET);
	}
	FLASH->CR &= ~FLASH_CR_PG;
	FLASH->CR |= FLASH_CR_LOCK;
}
