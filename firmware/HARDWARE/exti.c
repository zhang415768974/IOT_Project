#include "exti.h"
#include "sys.h"
#include "usart.h"
#include "../core.h"

void exti_init(void) {
	// GPIOB和辅助功能IO时钟使能
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
	GPIOB->CRH &= ~0xFFFFFFFF;
	// 设置GPIOB[8:15]为上/下拉输入模式(硬件io口外部已接10k上拉电阻)
	GPIOB->CRH |= 0x88888888;
	
	// 设置GPIOB[8:11]中断线
	AFIO->EXTICR[2] &= ~0xFFFF;
	AFIO->EXTICR[2] |= 0x1111;
	// 设置GPIOB[12:15]中断线
	AFIO->EXTICR[3] &= ~0xFFFF;
	AFIO->EXTICR[3] |= 0x1111;
	
	// 使能[8:15]中断
	EXTI->IMR |= 0xFF << 8;
	// 清除[8:15]中断触发模式
	EXTI->FTSR &= ~(0xFF << 8);
	// 设置[8:15]下降沿触发
	EXTI->FTSR |= 0xFF << 8;

	// 抢占优先级中00|00)
	NVIC_SetPriority(EXTI9_5_IRQn, 0x0);
	NVIC_SetPriority(EXTI15_10_IRQn, 0x0);
	
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	
	// 打开软件触发中断模式
	//SCB->CCR |= SCB_CCR_USERSETMPEND;
}


void EXTI9_5_IRQHandler() {
	u8 i;
	delay_ms(5);
	for (i = 8; i <= 9; ++i) {
		if ((GPIOB->IDR & (0x1 << i)) != RESET) {
			continue;
		}
		if (g_wait_sync == 0) {
			g_io_status ^= 1 << (i - 8);
			force_update_status(g_io_status);
			u1_printf("GPIOB[%02d]Trigger\r\n", i);
		} else {
			u1_printf("GPIOB[%02d]Suspend\r\n", i);
		}
		EXTI->PR |= 1 << i;
		break;
	}
}


void EXTI15_10_IRQHandler() {
	u8 i;
	delay_ms(5);
	for (i = 10; i <= 15; ++i) {
		if ((GPIOB->IDR & (0x1 << i)) != RESET) {
			continue;
		}
		if (g_wait_sync == 0) {
			g_io_status ^= 1 << (i - 8);
			force_update_status(g_io_status);
			u1_printf("GPIOB[%02d]Trigger\r\n", i);
		} else {
			u1_printf("GPIOB[%02d]Suspend\r\n", i);
		}
		EXTI->PR |= 1 << i;
		break;
	}
}
