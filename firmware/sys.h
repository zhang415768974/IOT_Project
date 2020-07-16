#ifndef __SYS_H__
#define __SYS_H__

void delay(unsigned short ms);
void led_init(void);
void u1_init(void);
void u2_init(void);
void u1_printf(char* fmt, ...);
void u2_printf(char* fmt, ...);
void tim3_init(unsigned short arr, unsigned short psc);
void exti_init(void);


void flash_write(unsigned int addr, const char* buf, unsigned short len);

void esp8266_init(void);
unsigned char esp8266_cmd(const char* cmd, const char* ack, unsigned short waittime, char* output);
void esp8266_send(const char* data, char* output);

#endif
