#ifndef __SYS_H__
#define __SYS_H__


#define SIGNATURE_BUF_SIZE 64

void delay(unsigned short ms);
void led_init(void);
void u1_init(void);
void u2_init(void);
void u1_printf(char* fmt, ...);
void u2_printf(char* fmt, ...);
void tim3_init(unsigned short arr, unsigned short psc);
void tim4_init(unsigned short arr, unsigned short psc);
void exti_init(void);


void flash_write(unsigned int addr, const char* buf, unsigned short len);

void esp8266_init(void);
unsigned char esp8266_cmd(const char* cmd, const char* ack, unsigned short waittime, char* output);
void iot_update(char* output);


enum NetStatus {
	WIFI_NoConnected,
	WIFI_Connecting,
	WIFI_Connected
};

#endif
