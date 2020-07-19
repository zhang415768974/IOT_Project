#ifndef __SYS_H__
#define __SYS_H__

#include "typedef.h"

#define SIGNATURE_BUF_SIZE 64


void delay_ms(u16 ms);
void led_init(void);
void iwdg_init(void);
void u1_init(void);
void u2_init(void);
void u1_printf(char* fmt, ...);
void u2_printf(char* fmt, ...);

void tim2_init(u16 arr, u16 psc);
void tim3_init(u16 arr, u16 psc);
void tim4_init(u16 arr, u16 psc);
void exti_init(void);

u8 check_init(void);


void led_double(u16 interval);
void flash_write(u32 addr, const char* buf, u16 len);

void esp8266_init(void);
u8 esp8266_cmd(const char* cmd, const char* ack, u16 waittime, const char** output);

void force_update_status(short status);
void build_getstatus_package(char* buf, u16 buf_size);
void build_setstatus_package(char* buf, u8 new_status, u16 buf_size);
void send_message(char* databuf, u16 databuf_size);


enum NetStatus {
	WIFI_NoInit,
	WIFI_Connecting,
	WIFI_Connected
};

#endif
