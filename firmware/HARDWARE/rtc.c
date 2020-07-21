#include "rtc.h"


void init_rtc(u32 timestamp) {
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR |= PWR_CR_DBP;
	RCC->BDCR |= RCC_BDCR_BDRST;
	RCC->BDCR &= ~RCC_BDCR_BDRST;
	
	RCC->BDCR |= RCC_BDCR_LSEON;
	while ((RCC->BDCR & RCC_BDCR_LSERDY) == RESET);
	
	RCC->BDCR |= RCC_BDCR_RTCSEL_LSE;
	RCC->BDCR |= RCC_BDCR_RTCEN;
	while ((RTC->CRL & RTC_CRL_RTOFF) == RESET);
	while ((RTC->CRL & RTC_CRL_RSF) == RESET);
	
	RTC->CRL |= RTC_CRL_CNF;
	RTC->PRLH = 0;
	RTC->PRLL = 32767;
	RTC->CNTL = timestamp & 0xFFFF;
	RTC->CNTH = timestamp >> 16;
	RTC->CRL &= ~RTC_CRL_CNF;
	while ((RTC->CRL & RTC_CRL_RTOFF) == RESET);
}
