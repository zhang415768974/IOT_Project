#include "timer.h"

void tim2_init(u16 arr, u16 psc, u8 priority) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->ARR = arr;
	TIM2->PSC = psc;
	TIM2->DIER = TIM_DIER_UIE;
	// 设置定时器中断
	NVIC_EnableIRQ(TIM2_IRQn);
	// 配置中断优先级
	NVIC_SetPriority(TIM2_IRQn, priority);
	TIM2->CR1 = TIM_CR1_CEN;
}


void tim3_init(u16 arr, u16 psc, u8 priority) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->ARR = arr;
	TIM3->PSC = psc;
	TIM3->DIER = TIM_DIER_UIE;
	// 设置定时器中断
	NVIC_EnableIRQ(TIM3_IRQn);
	// 配置中断优先级
	NVIC_SetPriority(TIM3_IRQn, priority);
	TIM3->CR1 = TIM_CR1_CEN;
}


void tim4_init(u16 arr, u16 psc, u8 priority) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	TIM4->ARR = arr;
	TIM4->PSC = psc;
	TIM4->DIER = TIM_DIER_UIE;
	// 设置定时器中断
	NVIC_EnableIRQ(TIM4_IRQn);
	// 配置中断优先级
	NVIC_SetPriority(TIM4_IRQn, priority);
	TIM4->CR1 = TIM_CR1_CEN;
}


void TIM2_IRQHandler() {
}


void TIM3_IRQHandler() {
}
