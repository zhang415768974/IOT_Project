#ifndef __DMA_H__
#define __DMA_H__

#include <stm32f10x.h>

void dma_config(DMA_Channel_TypeDef* DMA_CHx, u32 cpar, u32 cmar, u8 priority);
void dma_enable(DMA_Channel_TypeDef* DMA_CHx, u16 cndtr);

#endif
