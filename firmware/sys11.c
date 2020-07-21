#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stm32f10x.h>

#include "sys.h"
#include "cmd.h"
#include "md5.h"

#define UART_BUF_SIZE 256
#define SEND_BUF_SIZE 256
#define RECV_BUF_SIZE 256
#define PACKAGE_DATA_SIZE 256
#define MAX_CONNECT_TIMES 5


static u8 NET_STATUS;
u8 wait_sync;
extern u8 io_status;

/*
static void uart_dma_config(DMA_Channel_TypeDef* DMA_CHx, u32 cpar, u32 cmar, u8 priority) {
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	delay_ms(1);
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


static void uart_dma_enable(DMA_Channel_TypeDef* DMA_CHx, u16 len) {
	DMA_CHx->CCR &= ~DMA_CCR1_EN;
	DMA_CHx->CNDTR = len;
	DMA_CHx->CCR |= DMA_CCR1_EN;
}
*/

void force_update_status(short status) {
	u8 i;
	if (status >= 0 && NET_STATUS == WIFI_Connected) {
		wait_sync = 1;
		// 手动触发中断进行数据同步
		NVIC->STIR = TIM4_IRQn;
	}
	if (wait_sync == 1) {
		// 有需要等待的同步数据，等待其它定时器将数据同步完成后再MCU更新IO状态
		return;
	}
	for (i = 0; i != 8; ++i) {
		if ((io_status >> i) & 0x1) {
			GPIOB->ODR &= ~(1 << i);
		} else {
			GPIOB->ODR |= 1 << i;
		}
	}
}


/*
void led_double(u16 interval) {
	u8 i;
	for (i = 0; i != 2; ++i) {
		GPIOA->BSRR |= 1 << 6;
		delay_ms(interval);
		GPIOA->BSRR |= 1 << 22;
		delay_ms(interval);
	}
}


void u1_printf(char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(USART1_TX_BUF, UART_BUF_SIZE, fmt, ap);
	va_end(ap);
	//uart_dma_enable(DMA1_Channel4, strlen((const char*)USART1_TX_BUF));
	while (DMA1_Channel4->CNDTR != 0);
}



void u2_printf(char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(USART2_TX_BUF, UART_BUF_SIZE, fmt, ap);
	va_end(ap);
	//uart_dma_enable(DMA1_Channel7, strlen((const char*)USART2_TX_BUF));
	while (DMA1_Channel7->CNDTR != 0);
}


void iwdg_init(void) {
	IWDG->KR = 0x5555;
	IWDG->PR = 4;
	IWDG->RLR = 3125;
	IWDG->KR = 0xAAAA;
	IWDG->KR = 0xCCCC;
}


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
	u1_printf("timestamp init %u\r\n", timestamp);
}


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

	//设置优先级分组，2抢占，2响应
	NVIC_SetPriorityGrouping(2);
}


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


void TIM2_IRQHandler() {
	if (TIM2->SR & TIM_SR_UIF) {
		switch (NET_STATUS) {
			case WIFI_NoInit: {
				led_double(1000);
				break;
			}
			case WIFI_Connecting: {
				led_double(10);
				break;
			}
		}
		TIM2->SR &= ~TIM_SR_UIF;
	}
}


void TIM3_IRQHandler() {
	if (TIM3->SR & TIM_SR_UIF) {
		IWDG->KR = 0xAAAA;
		force_update_status(-1);
		TIM3->SR &= ~TIM_SR_UIF;
	}
}


void TIM4_IRQHandler() {
	u8 op_code;
	char buf[PACKAGE_DATA_SIZE];
	if (TIM4->SR & TIM_SR_UIF) {
		TIM4->SR &= ~TIM_SR_UIF;
		if (NET_STATUS != WIFI_Connected) {
			return;
		}
		memset(buf, 0, PACKAGE_DATA_SIZE);
		// 查看有没挂起的按键IO状态需要将当前状态同步到服务器
		if (wait_sync == 1) {
			build_setstatus_package(buf, (u8)io_status, PACKAGE_DATA_SIZE);
		} else {
			build_getstatus_package(buf, PACKAGE_DATA_SIZE);
		}
		send_message(buf, PACKAGE_DATA_SIZE);
		if (strlen(buf) >= PACKAGE_DATA_SIZE) {
			return;
		}
		if (strstr(buf, "+IPD")) {
			op_code = process_message(buf);
			if (op_code == 1) {
				u1_printf("get io_status ok\r\n");
			} else if (op_code == 2) {
				u1_printf("set io_status ok\r\n");
				wait_sync = 0;
				force_update_status(-1);
			}
		} else {
			u1_printf("error:\r\n\033[35m%s\033[0m\r\n", buf);
		}
	}
}


void USART1_IRQHandler() {
	static char recv_buf_data[UART_BUF_SIZE];
	static u8 index;
	char c;
	if (USART1->SR & USART_SR_RXNE) {
		c = (u8)USART1->DR;
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



void EXTI9_5_IRQHandler() {
	u8 i;
	delay_ms(5);
	for (i = 8; i <= 9; ++i) {
		if ((GPIOB->IDR & (0x1 << i)) != RESET) {
			continue;
		}
		if (wait_sync == 0) {
			io_status ^= 1 << (i - 8);
			force_update_status(io_status);
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
		if (wait_sync == 0) {
			io_status ^= 1 << (i - 8);
			force_update_status(io_status);
			u1_printf("GPIOB[%02d]Trigger\r\n", i);
		} else {
			u1_printf("GPIOB[%02d]Suspend\r\n", i);
		}
		EXTI->PR |= 1 << i;
		break;
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
	
	// 打开软件触发中断模式
	SCB->CCR |= SCB_CCR_USERSETMPEND;
}


void tim2_init(u16 arr, u16 psc) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->ARR = arr;
	TIM2->PSC = psc;
	TIM2->DIER = TIM_DIER_UIE;
	// 设置定时器中断
	NVIC->ISER[TIM2_IRQn / 32] |= 1 << (TIM2_IRQn % 32);
	// 抢占优先级最低b1111
	NVIC_SetPriority(TIM2_IRQn, 0xF);
	TIM2->CR1 = TIM_CR1_CEN;
}


void tim3_init(u16 arr, u16 psc) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->ARR = arr;
	TIM3->PSC = psc;
	TIM3->DIER = TIM_DIER_UIE;
	// 设置定时器中断
	NVIC->ISER[TIM3_IRQn / 32] |= 1 << (TIM3_IRQn % 32);
	// 抢占优先级中b1000
	NVIC_SetPriority(TIM3_IRQn, 0x8);
	TIM3->CR1 = TIM_CR1_CEN;
}


void tim4_init(u16 arr, u16 psc) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	TIM4->ARR = arr;
	TIM4->PSC = psc;
	TIM4->DIER = TIM_DIER_UIE;
	// 设置定时器中断
	NVIC->ISER[TIM4_IRQn / 32] |= 1 << (TIM4_IRQn % 32);
	// 抢占优先级中b1001
	NVIC_SetPriority(TIM4_IRQn, 0x9);
	TIM4->CR1 = TIM_CR1_CEN;
}


void led_init() {
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
	// 抢占优先级中1001(低于按键和喂狗)
	NVIC_SetPriority(USART1_IRQn, 0x9);
	USART1->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_PEIE;
	USART1->CR3 |= USART_CR3_DMAT;
	//uart_dma_config(DMA1_Channel4, (u32)&USART1->DR, (u32)&USART1_TX_BUF, 1);
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
	uart_dma_config(DMA1_Channel7, (u32)&USART2->DR, (u32)&USART2_TX_BUF, 3);
	USART2_RX_COUNT = 0;
	USART2->CR1 |= USART_CR1_UE;
}
*/

u8 esp8266_cmd(const char* cmd, const char* ack, u16 waittime, const char** output) {
	USART2_RX_COUNT = 0;
	u2_printf("%s\r\n", cmd);
	if (ack && waittime) {
		while (--waittime) {
			delay_ms(1);
		}
	}
	USART2_RX_BUF[USART2_RX_COUNT] = 0;
	USART2_RX_COUNT = 0;
	if (output) {
		*output = (char*)&USART2_RX_BUF;
	}
	if (ack && strstr((const char*)USART2_RX_BUF, ack)) {
		return 1;
	}
	return 0;
}


void esp8266_init(void) {
	char temp[64];
	u8 times;
	const char* output;
	if (NET_STATUS == WIFI_Connecting || NET_STATUS == WIFI_Connected) {
		return;
	}
	u1_printf("设置esp8266为STA模式\r\n", NET_STATUS);
	NET_STATUS = WIFI_Connecting;
	esp8266_cmd("AT+CWMODE=1", "OK", 200, NULL);
	//u1_printf("复位esp8266\r\n");
	//esp8266_cmd("AT+RST", NULL, 10000, NULL);
	times = 1;
	do {
		u1_printf("开始第%u次连接无线网络[%s]\r\n", times, iot_data->wifi_name);
		snprintf(temp, 64, "AT+CWJAP=\"%s\",\"%s\"", iot_data->wifi_name, iot_data->wifi_password);
		if (esp8266_cmd(temp, "OK", 5000, NULL) != 1) {
			if (times++ > MAX_CONNECT_TIMES) {
				u1_printf("网络初始化失败,请手动检查网络情况\r\n");
				return;
			}
			continue;
		}
		u1_printf("设置esp8266非透传模式\r\n");	
		if (esp8266_cmd("AT+CIPMODE=0", "OK", 200, NULL) != 1) {
			continue;
		}
		u1_printf("设置esp8266单播模式\r\n");
		if (esp8266_cmd("AT+CIPMUX=0", "OK", 200, NULL) != 1) {
			continue;
		}
		u1_printf("查询esp8266网络信息\r\n");
		if (esp8266_cmd("AT+CIFSR", "OK", 200, &output) != 1) {
			continue;
		}
		u1_printf("%s\r\n", output);
		break;
	} while (1);
	NET_STATUS = WIFI_Connected;
	u1_printf("\r\n\033[36;1mDone\033[0m\r\n");
}


void build_getstatus_package(char* buf, u16 buf_size) {
	char signature_data[SIGNATURE_BUF_SIZE];
	char token_data[MD5_HASHSIZE * 2 + 1];
	u32 timestamp;

	timestamp = (RTC->CNTL & 0xFFFF) | ((RTC->CNTH & 0xFFFF) << 16);
	snprintf(signature_data, SIGNATURE_BUF_SIZE,
		"iot#1#%s#%u#%u#%s",
		iot_data->machineid,
		iot_data->customerid,
		timestamp,
		iot_data->secret_key
	);
	memset(token_data, 0, sizeof(token_data));
	md5_hexdigest(signature_data, strlen(signature_data), token_data);
	snprintf(buf, buf_size,
		"data=iot#1#%s#%u#%u#%s#eof",
		iot_data->machineid,
		iot_data->customerid,
		timestamp,
		token_data
	);
	timestamp++;
}


void build_setstatus_package(char* buf, u8 new_status, u16 buf_size) {
	char signature_data[SIGNATURE_BUF_SIZE];
	char token_data[MD5_HASHSIZE * 2 + 1];
	static u32 timestamp;

	//timestamp = 123456;
	snprintf(signature_data, SIGNATURE_BUF_SIZE,
		"iot#2#%s#%u#%u#%u#%s",
		iot_data->machineid,
		iot_data->customerid,
		timestamp,
		new_status,
		iot_data->secret_key
	);
	memset(token_data, 0, sizeof(token_data));
	md5_hexdigest(signature_data, strlen(signature_data), token_data);
	snprintf(buf, buf_size,
		"data=iot#2#%s#%u#%u#%u#%s#eof",
		iot_data->machineid,
		iot_data->customerid,
		timestamp,
		new_status,
		token_data
	);
	timestamp++;
}


void send_message(char* databuf, u16 databuf_size) {
	char temp[64];
	char post_data[SEND_BUF_SIZE];
	u16 len;
	const char* result;
	if (NET_STATUS != WIFI_Connected) {
		return;
	}
	snprintf(post_data, SEND_BUF_SIZE,
		"POST / HTTP/1.1\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"User-Agent: iot_esp8266_module\r\n"
		"Content-Length:%d\r\n"
		"\r\n%s", strlen(databuf), databuf);

	snprintf(temp, 64, "AT+CIPSTART=\"TCP\",\"%s\",%d", iot_data->server_ip, iot_data->server_port);
	if (esp8266_cmd(temp, "CONNECT", 1000, NULL) == 1) {
		u1_printf("连接服务器[%s:%u]成功 \r\n", iot_data->server_ip, iot_data->server_port);
	}

	len = strlen(post_data);
	snprintf(temp, 64, "AT+CIPSEND=%d", len);
	esp8266_cmd(temp, "OK", 200, NULL);

	u1_printf("\033[0msendto >>>>>>\r\n\033[32m%s\033[0m \r\n", post_data);
	esp8266_cmd(post_data, "iot", 1000, &result);
	len = strlen(result);

	if (len < databuf_size - 1) {
		u1_printf("recvfrom <<<<<<\r\n\033[34m%s\033[0m \r\n", result);
		strncpy(databuf, result, len);
	}
	if (esp8266_cmd("AT+CIPCLOSE", "OK", 200, NULL) == 1) {
		u1_printf("\r\n\033[0m通信结束,断开与服务器的连接 \r\n");
	}
}


u8 check_init(void) {
	u8 isok;
	isok = 1;
	NET_STATUS = WIFI_NoInit;
	if ((u8)iot_data->model[0] == 0xFF) {
		u1_printf("need setmodel\r\n");
		isok = 0;
	}
	if ((u8)iot_data->wifi_name[0] == 0xFF) {
		u1_printf("need setwifiname\r\n");
		isok = 0;
	}
	if ((u8)iot_data->wifi_password[0] == 0xFF) {
		u1_printf("need setwifipwd\r\n");
		isok = 0;
	}
	if ((u8)iot_data->server_ip[0] == 0xFF) {
		u1_printf("need setserverip\r\n");
		isok = 0;
	}
	if ((u8)iot_data->secret_key[0] == 0xFF) {
		u1_printf("need setkey\r\n");
		isok = 0;
	}
	if ((u8)iot_data->machineid[0] == 0xFF) {
		u1_printf("need setmid\r\n");
		isok = 0;
	}
	if (iot_data->customerid == 0xFFFFFFFF) {
		u1_printf("need setcid\r\n");
		isok = 0;
	}
	if (iot_data->server_port == 0xFFFF) {
		u1_printf("need setserverport\r\n");
		isok = 0;
	}
	return isok;
}
