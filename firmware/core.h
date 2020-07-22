#ifndef __CORE_H__
#define __CORE_H__

#include <stm32f10x.h>

enum NetStatus {
	WIFI_NoInit,
	WIFI_Connecting,
	WIFI_Connected
};


extern u8 g_wait_sync;	// 等待io_status状态更新
extern u8 g_io_status;	// 当前模块io_status值
extern u8 g_net_status;	// 当前网络状态

void iot_init(void);
void iot_run(void);
u8 check_init(void);

u8 iot_connect_wifi(const char* ssid, const char* passwd);

void force_update_status(short status);


void build_getstatus_package(char* buf, u16 buf_size);
void build_setstatus_package(char* buf, u8 new_status, u16 buf_size);
u8 send_message(char* databuf, u16 databuf_size);

#endif
