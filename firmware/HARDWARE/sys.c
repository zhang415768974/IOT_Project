#include "sys.h"

void delay_ms(u16 ms) {
	u32 temp;
	SysTick->LOAD = 9000 * ms;
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE;
	do {
		temp = SysTick->CTRL;
	} while ((temp & SysTick_CTRL_ENABLE) && !(temp & SysTick_CTRL_COUNTFLAG_Msk));
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE;
}


void SystemInit() {
	// 外部高速时钟使能
	RCC->CR |= RCC_CR_HSEON;
	// 等待外部高速时钟就绪
	while ((RCC->CR & RCC_CR_HSERDY) == RESET);
	
	// 设置低速总线时钟2分频,高速总线时钟不分频,系统时钟不分频
	RCC->CFGR = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_HPRE_DIV1;
	// 设置锁相环倍频系数9倍,时钟来源外部8M晶振
	RCC->CFGR |= RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLSRC;
	
	// 全速运行(72M主频)FLASH需要等待2个时钟周期
	FLASH->ACR |= FLASH_ACR_LATENCY_2;
	
	// 使能锁相环
	RCC->CR |= RCC_CR_PLLON;
	// 等待锁相环就绪
	while ((RCC->CR & RCC_CR_PLLRDY) == RESET);
	
	// 切换锁相环为系统时钟
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	// 等待系统时钟切换就绪
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
	
	// 设置系统定时器使用外部时钟源
	SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE;

	/*设置优先级分组，2抢占，2响应*/
	NVIC_SetPriorityGrouping(2);
}


void iwdg_init(void) {
	IWDG->KR = 0x5555;
	IWDG->PR = 5; // 看门狗128分频
	IWDG->RLR = 3125;
	IWDG->KR = 0xAAAA;
	IWDG->KR = 0xCCCC;
}


void led_init(void) {
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
	GPIOA->CRL &= ~(0xF << 24);
	GPIOA->CRL |= 2 << 24;
	
	// 取消JLINK的部分端口调试复用
	AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG;
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
	
	GPIOB->CRL &= ~0xFFFFFFFF;
	GPIOB->CRL |= 0x22222222;
	GPIOB->ODR |= 0xFF;
}


void led_double(u16 interval) {
	u8 i;
	for (i = 0; i != 2; ++i) {
		GPIOA->BSRR |= 1 << 6;
		delay_ms(interval);
		GPIOA->BSRR |= 1 << 22;
		delay_ms(interval);
	}
}
