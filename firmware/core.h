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


void force_update_status(short status);

#endif
