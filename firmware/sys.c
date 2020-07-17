#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stm32f10x.h>

#include "sys.h"
#include "cmd.h"
#include "md5.h"

#define UART_BUF_SIZE 512
#define SEND_BUF_SIZE 512
#define POST_BUF_SIZE 512
#define PACKAGE_DATA_SIZE 128

#define FLASH_KEY1			0x45670123
#define FLASH_KEY2			0xCDEF89AB	

__align(4) static char USART1_TX_BUF[UART_BUF_SIZE];
__align(4) static char USART2_TX_BUF[UART_BUF_SIZE];

static unsigned char USART2_RX_BUF[UART_BUF_SIZE];
static unsigned short USART2_RX_COUNT;
static unsigned char ESP8266_READY;

static void uart_dma_config(DMA_Channel_TypeDef* DMA_CHx, unsigned int cpar, unsigned int cmar, unsigned char priority) {
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	delay(1);
	DMA_CHx->CPAR = cpar;
	DMA_CHx->CMAR = cmar;
	DMA_CHx->CCR = 0;
	DMA_CHx->CCR |= 1 << 4;
	DMA_CHx->CCR &= ~(1 << 5);
	DMA_CHx->CCR &= ~(1 << 6);
	DMA_CHx->CCR |= priority << 7; // 通道优先级
	DMA_CHx->CCR &= ~(0x3 << 8);
	DMA_CHx->CCR &= ~(0x3 << 10);
	DMA_CHx->CCR |= 1 << 12;
	DMA_CHx->CCR &= ~(0x1 << 14);
}


static void uart_dma_enable(DMA_Channel_TypeDef* DMA_CHx, unsigned short len) {
	DMA_CHx->CCR &= ~DMA_CCR1_EN;
	DMA_CHx->CNDTR = len;
	DMA_CHx->CCR |= DMA_CCR1_EN;
}


void u1_printf(char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(USART1_TX_BUF, UART_BUF_SIZE, fmt, ap);
	va_end(ap);
	uart_dma_enable(DMA1_Channel4, strlen((const char*)USART1_TX_BUF));
	while (DMA1_Channel4->CNDTR != 0);
}


void u2_printf(char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(USART2_TX_BUF, UART_BUF_SIZE, fmt, ap);
	va_end(ap);
	uart_dma_enable(DMA1_Channel7, strlen((const char*)USART2_TX_BUF));
	while (DMA1_Channel7->CNDTR != 0);
}


void flash_write(unsigned int addr, const char* buf, unsigned short size) {
	unsigned int secpos;
	unsigned short i;
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
		*(unsigned short*)secpos = *((unsigned short*)buf + i);
		secpos += 2;
		while ((FLASH->SR & FLASH_SR_BSY) != RESET);
	}
	FLASH->CR &= ~FLASH_CR_PG;
	FLASH->CR |= FLASH_CR_LOCK;
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
}


void delay(unsigned short ms) {
	unsigned int temp;
	SysTick->LOAD = 9000 * ms;
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE;
	do {
		temp = SysTick->CTRL;
	} while ((temp & SysTick_CTRL_ENABLE) && !(temp & SysTick_CTRL_COUNTFLAG_Msk));
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE;
}


void TIM3_IRQHandler() {
	uint8_t i;
	if (TIM3->SR & TIM_SR_UIF) {
		for (i = 0; i != 2; ++i){
			GPIOA->BSRR |= 1 << 6;
			delay(80);
			GPIOA->BSRR |= 1 << 22;
			delay(80);
		}
		TIM3->SR &= ~TIM_SR_UIF;
	}
}


void TIM4_IRQHandler() {
	
	if (TIM4->SR & TIM_SR_UIF) {
		if (ESP8266_READY == WIFI_Connected) {
			
		}
		TIM4->SR &= ~TIM_SR_UIF;
	}
}


void USART1_IRQHandler() {
	static char recv_buf_data[UART_BUF_SIZE];
	static unsigned char index = 0;
	char c;
	if (USART1->SR & USART_SR_RXNE) {
		c = (unsigned char)USART1->DR;
		if (c != '\r') {
			if (c == '\b') {
				if (index != 0) {
					recv_buf_data[index--] = 0;
					u1_printf("\b \b");
				}
			} else {
				if (index < UART_BUF_SIZE - 1) {
					recv_buf_data[index++] = c;
					u1_printf("%c", c);
				}
			}
		} else {
			recv_buf_data[index] = 0;
			if (0 == index) {
				u1_printf("\r\n=> ");
				return;
			}
			index = 0;
			u1_printf("\r\n");
			process_cmdline(recv_buf_data);
			memset(recv_buf_data, 0, UART_BUF_SIZE);
			u1_printf("=> ");
		}
	}
}


void USART2_IRQHandler() {
	if ((USART2->SR & USART_SR_RXNE) &&
		USART2_RX_COUNT < UART_BUF_SIZE - 1) {
		USART2_RX_BUF[USART2_RX_COUNT++] = USART2->DR & 0xFF;
	}
}


void EXTI9_5_IRQHandler() {
	uint8_t i;
	delay(5);
	for (i = 8; i <= 9; ++i) {
		if ((GPIOB->IDR & (0x1 << i)) == RESET) {
			GPIOB->ODR ^= 1 << (i - 8);
			u1_printf("GPIOB[%02d]Trigger\r\n", i);
		}
		EXTI->PR |= 1 << i;
	}
}


void EXTI15_10_IRQHandler() {
	uint8_t i;
	delay(5);
	for (i = 10; i <= 15; ++i) {
		if ((GPIOB->IDR & (0x1 << i)) == RESET) {
			GPIOB->ODR ^= 1 << (i - 8);
			u1_printf("GPIOB[%02d]Trigger\r\n", i);
		}
		EXTI->PR |= 1 << i;
	}
}


void exti_init() {
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
	// 抢占优先级中1000(高于串口1)
	NVIC_SetPriority(EXTI9_5_IRQn, 0x8);
	NVIC->ISER[EXTI9_5_IRQn / 32] |= 1 << (EXTI9_5_IRQn % 32);
	NVIC_SetPriority(EXTI15_10_IRQn, 0x8);
	NVIC->ISER[EXTI15_10_IRQn / 32] |= 1 << (EXTI15_10_IRQn % 32);
}


void tim3_init(unsigned short arr, unsigned short psc) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->ARR = arr;
	TIM3->PSC = psc;
	TIM3->DIER = TIM_DIER_UIE;
	// 设置定时器中断
	NVIC->ISER[TIM3_IRQn / 32] |= 1 << (TIM3_IRQn % 32);
	// 抢占优先级最低1100
	NVIC_SetPriority(TIM3_IRQn, 0xC);
	TIM3->CR1 = TIM_CR1_CEN;
}


void tim4_init(unsigned short arr, unsigned short psc) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	TIM4->ARR = arr;
	TIM4->PSC = psc;
	TIM4->DIER = TIM_DIER_UIE;
	// 设置定时器中断
	NVIC->ISER[TIM4_IRQn / 32] |= 1 << (TIM4_IRQn % 32);
	// 抢占优先级最低1100
	NVIC_SetPriority(TIM4_IRQn, 0xC);
	TIM4->CR1 = TIM_CR1_CEN;
}


void led_init() {
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
	GPIOA->CRL &= ~(0xF << 24);
	GPIOA->CRL |= 2 << 24;
	
	AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG;
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
	
	GPIOB->CRL &= ~0xFFFFFFFF;
	GPIOB->CRL |= 0x22222222;
	GPIOB->ODR |= 0xFF;
}


void u1_init() {
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
	GPIOA->CRH &= ~(0xFF << 4);
	GPIOA->CRH |= 0x8A << 4;
	RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
	
	// 波特率: 72000000 / 115200 / 16 = 39.0625
	USART1->BRR = (39 << 4) | (int)(0.0625 * 16);
	// 设置串口收数据中断
	NVIC->ISER[USART1_IRQn / 32] |= 1 << (USART1_IRQn % 32);
	// 抢占优先级中1001(低于按键)
	NVIC_SetPriority(USART1_IRQn, 0x9);
	USART1->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_PEIE;
	USART1->CR3 |= USART_CR3_DMAT;
	uart_dma_config(DMA1_Channel4, (unsigned int)&USART1->DR, (unsigned int)&USART1_TX_BUF, 1);
	USART1->CR1 |= USART_CR1_UE;
}


void u2_init() {
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	GPIOA->CRL &= ~(0xFF << 8);
	GPIOA->CRL |= 0x8A << 8;
	RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;
	USART2->BRR = 19 << 4;
	NVIC->ISER[USART2_IRQn / 32] |= 1 << (USART2_IRQn % 32);
	// 抢占优先级高0100
	NVIC_SetPriority(USART1_IRQn, 0x4);
	USART2->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;
	USART2->CR3 |= USART_CR3_DMAT;
	uart_dma_config(DMA1_Channel7, (unsigned int)&USART2->DR, (unsigned int)&USART2_TX_BUF, 3);
	USART2_RX_COUNT = 0;
	USART2->CR1 |= USART_CR1_UE;
}


unsigned char esp8266_cmd(const char* cmd, const char* ack, unsigned short waittime, char* output) {
	delay(20);
	USART2_RX_COUNT = 0;
	u2_printf("%s\r\n", cmd);
	if (ack && waittime) {
		while (--waittime) {
			delay(1);
		}
	}
	USART2_RX_BUF[USART2_RX_COUNT] = 0;
	if (output) {
		strncpy(output, (const char*)USART2_RX_BUF, strlen((const char*)USART2_RX_BUF));
	}
	if (ack && strstr((const char*)USART2_RX_BUF, ack)) {
		return 1;
	}
	return 0;
}


void esp8266_init(void) {
	char buf[128];
	if (ESP8266_READY == WIFI_Connecting || ESP8266_READY == WIFI_Connected) {
		return;
	}
	GPIOA->BSRR |= 1 << 22;
	ESP8266_READY = WIFI_Connecting;
	u1_printf("设置esp8266为STA模式\r\n");
	esp8266_cmd("AT+CIPMODE=0", "OK", 200, NULL);
	if (esp8266_cmd("AT+CWMODE=1", "OK", 200, NULL) != 1) {
		ESP8266_READY = WIFI_NoConnected;
		return;
	}
	//u1_printf("复位esp8266\r\n");
	//esp8266_cmd("AT+RST", "version", 2000, NULL);
	//delay(1000);
	u1_printf("开始连接无线网络[%s]\r\n", iot_data->wifi_name);
	sprintf(buf, "AT+CWJAP=\"%s\",\"%s\"", iot_data->wifi_name, iot_data->wifi_password);
	if (esp8266_cmd(buf, "OK", 5000, NULL) != 1) {
		ESP8266_READY = WIFI_NoConnected;
		return;
	}
	u1_printf("设置esp8266非透传模式\r\n");	
	esp8266_cmd("AT+CIPMODE=0", "OK", 200, NULL);
	u1_printf("设置esp8266单播模式\r\n");
	esp8266_cmd("AT+CIPMUX=0", "OK", 200, NULL);
	u1_printf("查询esp8266网络信息\r\n");
	if (esp8266_cmd("AT+CIFSR", "OK", 200, buf) != 1) {
		ESP8266_READY = WIFI_NoConnected;
		return;
	}
	u1_printf("%s\r\n", buf);
	ESP8266_READY = WIFI_Connected;
	u1_printf("Done\r\n");
}


void request_status_package(char* output) {
	char signature_data[SIGNATURE_BUF_SIZE];
	char token_data[MD5_HASHSIZE * 2 + 1];
	unsigned int timestamp;

	timestamp = 123456;
	sprintf(signature_data, "iot#1#%s#%u#%u#%s",
		iot_data->machineid,
		iot_data->customerid,
		timestamp,
		iot_data->secret_key
	);
	memset(token_data, 0, sizeof(token_data));
	md5_hexdigest(signature_data, strlen(signature_data), token_data);
	sprintf(output, "data=iot#1#%s#%u#%u#%s#eof",
		iot_data->machineid,
		iot_data->customerid,
		timestamp,
		token_data
	);
}


void iot_update(char* output) {
	char temp[128];
	char data_package[PACKAGE_DATA_SIZE];
	char post_data[SEND_BUF_SIZE];
	unsigned short len;
	
	if (ESP8266_READY == WIFI_NoConnected || ESP8266_READY == WIFI_Connecting) {
		esp8266_init();
		return;
	}
	if (ESP8266_READY == WIFI_Connecting) {
		return;
	}
	if (ESP8266_READY != WIFI_Connected) {
		return;
	}

	memset(data_package, 0, PACKAGE_DATA_SIZE);
	request_status_package(data_package);
	sprintf(post_data, "POST / HTTP/1.1\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"User-Agent: iot_esp8266_module\r\n"
		"Content-Length:%d\r\n"
		"\r\n%s", strlen(data_package), data_package);

	len = strlen(post_data);
	if (len > SEND_BUF_SIZE) {
		return;
	}
	sprintf(temp, "AT+CIPSTART=\"TCP\",\"%s\",%d", iot_data->server_ip, iot_data->server_port);
	if (esp8266_cmd(temp, "CONNECT", 1000, NULL) == 1) {
		GPIOA->BSRR |= 1 << 6;
		u1_printf("连接服务器[%s:%u]成功 \r\n", iot_data->server_ip, iot_data->server_port);
	}

	sprintf(temp, "AT+CIPSEND=%d", len);
	esp8266_cmd(temp, "OK", 200, NULL);
	
	esp8266_cmd(post_data, "iot", 1000, output);
	delay(200);
	//u1_printf("recv:%s\r\n", output);
	if (esp8266_cmd("AT+CIPCLOSE", "OK", 200, NULL) == 1) {
		delay(50);
		GPIOA->BSRR |= 1 << 22;
		u1_printf("通信结束,断开与服务器的连接 \r\n");
	}
}
