#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp8266.h"
#include "sys.h"
#include "usart.h"


void esp8266_init(void) {
	GPIOA->CRL &= ~(0xF << 28);
	GPIOA->CRL |= 0x2 << 28;
	GPIOA->ODR &= ~(1 << 7);
	delay_ms(10);
	GPIOA->ODR |= 1 << 7;
	delay_ms(200);
}


static u8* esp8266_check_cmd(const char *str) {
	if (USART2_RX_STA & 0x8000) {
		USART2_RX_BUF[USART2_RX_STA & 0x7FFF] = 0;	//添加结束符0
		return (u8*)strstr((const char*)USART2_RX_BUF, str);
	}
	return NULL;
}


u8 esp8266_send_cmd(const char *cmd, const char *ack, u16 waittime, char* output, u16 size) {
	u8 result = 0;
	u16 len;
	USART2_RX_STA = 0;
	if (strcmp(cmd, "+++") == 0) {
		u2_printf("+++");	// 退出透传模式特殊处理
		return 1;
	} else {
		u2_printf("%s\r\n", cmd);
	}
	if (ack && waittime) {
		while (--waittime) {
			delay_ms(10);
			if ((USART2_RX_STA & 0x8000) == RESET) {
				continue;
			}
			if (esp8266_check_cmd(ack)) {
				break;
			}
			USART2_RX_STA = 0;
		}
		if (waittime == 0) {
			result = 1;
		}
		if (output && size) {
			len = strlen((const char*)USART2_RX_BUF);
			if (len < size) {
				strncpy(output, (const char*)USART2_RX_BUF, len);
			}
		}
	}
	return result;
}
