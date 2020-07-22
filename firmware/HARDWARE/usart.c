#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "usart.h"
#include "dma.h"
#include "timer.h"
#include "../cmd.h"

// 串口1发送缓冲区
__align(8) u8 USART1_TX_BUF[USART_MAX_SEND_LEN];
// 串口2发送缓冲区
__align(8) u8 USART2_TX_BUF[USART_MAX_SEND_LEN];


// 串口1接收缓冲区
static u8 USART1_RX_BUF[USART_MAX_RECV_LEN];
// 串口2接收缓冲区
u8 USART2_RX_BUF[USART_MAX_RECV_LEN];

//[15]:0,没有收到数据的状态,1:接收到了一批数据
//[14:0]:接收到的数据长度
u16 USART2_RX_STA = 0;

void USART2_IRQHandler() {
	u8 res;
	if ((USART2->SR & USART_SR_RXNE) == RESET) {
		return;
	}
	res = (u8)USART2->DR;
	if (USART2_RX_STA < USART_MAX_RECV_LEN) {
		TIM4->CNT = 0;
		if (USART2_RX_STA == 0) {
			TIM4->CR1 |= TIM_CR1_CEN;
		}
		USART2_RX_BUF[USART2_RX_STA++] = res;
	} else {
		USART2_RX_STA |= 1 << 15;
	}
}


void USART1_IRQHandler() {
	static u16 index;
	char c;
	if (USART1->SR & USART_SR_RXNE) {
		c = (u8)USART1->DR;
		if (c != '\r') {
			if (c == '\b') {
				if (index != 0) {
					USART1_RX_BUF[index--] = 0;
					u1_printf("\b \b");
				}
			} else {
				if (index < USART_MAX_RECV_LEN - 1) {
					USART1_RX_BUF[index++] = c;
					u1_printf("%c", c);
				}
			}
		} else {
			USART1_RX_BUF[index] = 0;
			if (0 == index) {
				u1_printf("\r\n=> ");
				return;
			}
			index = 0;
			u1_printf("\r\n");
			dispatch_cmdline((const char*)USART1_RX_BUF);
			memset(USART1_RX_BUF, 0, USART_MAX_RECV_LEN);
			u1_printf("=> ");
		}
	}
}


// 初始化串口2
void u2_init(void) {
	// 使能串口2模块
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	// 配置功能复用引脚
	GPIOA->CRL &= ~(0xFF << 8);
	GPIOA->CRL |= 0x8A << 8;
	// 复位串口2
	RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;
	// 设置波特率115200
	USART2->BRR = 19 << 4;
	// 启用串口2中断
	NVIC_EnableIRQ(USART2_IRQn);
	// 配置中断优先级10|00
	NVIC_SetPriority(USART1_IRQn, 0x8);
	// 配置收发数据使能、非空和PE中断
	USART2->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_PEIE;
	// 配置DMA写数据
	USART2->CR3 |= USART_CR3_DMAT;
	dma_config(DMA1_Channel7, (u32)&USART2->DR, (u32)&USART2_TX_BUF, 1);
	// 配置定时器4控制收数据处理(10ms中断)
	tim4_init(999, 7199, 0x4); // 高中断01|00
	USART2_RX_STA = 0;
	// 打开串口2
	USART2->CR1 |= USART_CR1_UE;
	// 先关闭定时器4
	TIM4->CR1 &= ~TIM_CR1_CEN;
}


void u1_init(void) {
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
	GPIOA->CRH &= ~(0xFF << 4);
	GPIOA->CRH |= 0x8A << 4;
	RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
	
	// 波特率: 72000000 / 115200 / 16 = 39.0625
	USART1->BRR = (39 << 4) | (int)(0.0625 * 16);
	// 设置串口收数据中断
	NVIC_EnableIRQ(USART1_IRQn);
	// 抢占优先级中10|01
	NVIC_SetPriority(USART1_IRQn, 0x9);
	USART1->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_PEIE;
	USART1->CR3 |= USART_CR3_DMAT;
	dma_config(DMA1_Channel4, (u32)&USART1->DR, (u32)&USART1_TX_BUF, 1);
	USART1->CR1 |= USART_CR1_UE;
}


void u1_printf(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf((char*)USART1_TX_BUF, USART_MAX_SEND_LEN, fmt, ap);
	va_end(ap);
	dma_enable(DMA1_Channel4, strlen((const char*)USART1_TX_BUF));
	while (DMA1_Channel4->CNDTR != 0);
}


void u2_printf(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf((char*)USART2_TX_BUF, USART_MAX_SEND_LEN, fmt, ap);
	va_end(ap);
	dma_enable(DMA1_Channel7, strlen((const char*)USART2_TX_BUF));	// 通过dma把数据发出去
	while (DMA1_Channel7->CNDTR != 0); // 等待通道7传输完成
}

void TIM4_IRQHandler() {
	if ((TIM4->SR & TIM_SR_UIF) == RESET) {
		return;
	}
	USART2_RX_STA |= 1 << 15;	// 标记接收完成
	TIM4->SR &= ~TIM_SR_UIF;	// 清除中断标记位
	TIM4->CR1 &= ~TIM_CR1_CEN;	// 关闭定时器4
}
