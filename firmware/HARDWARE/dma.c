#include "dma.h"
#include "sys.h"

void dma_config(DMA_Channel_TypeDef* DMA_CHx, u32 cpar, u32 cmar, u8 priority) {
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	delay_ms(5);
	DMA_CHx->CPAR = cpar;
	DMA_CHx->CMAR = cmar;
	DMA_CHx->CCR = 0;
	DMA_CHx->CCR |= 1 << 4;	// 从存储器读
	DMA_CHx->CCR &= ~(1 << 5);	// 普通模式
	DMA_CHx->CCR &= ~(1 << 6);	// 外设地址非增量模式
	DMA_CHx->CCR |= 1 << 7;		// 存储器增量模式
	DMA_CHx->CCR &= ~(0x3 << 8);	// 外设数据宽度为8位
	DMA_CHx->CCR &= ~(0x3 << 10);	// 存储器数据宽度为8位
	DMA_CHx->CCR |= 1 << 12;	// 中等优先级
	DMA_CHx->CCR &= ~(1 << 14);	// 非存储器到存储器模式
}

void dma_enable(DMA_Channel_TypeDef* DMA_CHx, u16 cndtr) {
	DMA_CHx->CCR &= ~DMA_CCR1_EN;
	DMA_CHx->CNDTR = cndtr;
	DMA_CHx->CCR |= DMA_CCR1_EN;
}
