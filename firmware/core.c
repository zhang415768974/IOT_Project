#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "sys.h"
#include "timer.h"
#include "usart.h"
#include "rtc.h"
#include "exti.h"
#include "esp8266.h"

#include "md5.h"


u8 g_wait_sync = 0;
u8 g_io_status;
u8 g_net_status;

static void iot_update(void) {
	// 喂狗
	IWDG->KR = 0xAAAA;
	// IO状态更新
}


void iot_init() {
	// 定时器1秒一次看门狗喂狗,5秒不喂自动复位MCU
	//iwdg_init();
	// 指示灯初始化
	led_init();
	// 串口1控制台初始化
	u1_init();
	// 按键中断初始化
	exti_init();
	// RTC时钟初始化
	init_rtc(0);
	// esp8266初始化
	esp8266_init();
}


void iot_run(void) {
	for (; ;) {
		delay_ms(1000);
		iot_update();
	}
}


void force_update_status(short status) {
	u8 i;
	if (status > 0 && g_net_status == WIFI_Connected) {
		// 手动立即触发更新
		g_wait_sync = 1;
	}
	// 判断是否等待其它定时器将数据同步完成后再MCU更新IO状态
	if (g_wait_sync == 1) {
		return;
	}
	for (i = 0; i != 8; ++i) {
		if ((g_io_status >> i) & 0x1) {
			GPIOB->ODR &= ~(1 << i);
		} else {
			GPIOB->ODR |= 1 << i;
		}
	}
}
