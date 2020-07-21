#ifndef __ESP8266_H__
#define __ESP8266_H__

#include <stm32f10x.h>

void esp8266_init(void);

u8 esp8266_send_cmd(const char *cmd, const char *ack, u16 waittime, char* output, u16 len);

#endif
