#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "sys.h"
#include "timer.h"
#include "usart.h"
#include "rtc.h"
#include "exti.h"
#include "cmd.h"
#include "esp8266.h"

#include "md5.h"


#define MAX_CONNECT_TIMES	5		// 网络连接最大尝试次数
#define PACKAGE_DATA_SIZE	512   // 最大数据包大小
#define SIGNATURE_BUF_SIZE	128	// 用于构造签名的缓冲区大小

u8 g_wait_sync = 0;
u8 g_io_status;
u8 g_net_status;


void iot_init() {
	// 初始化看门狗,10秒不喂自动复位MCU
	iwdg_init();
	// 指示灯初始化
	led_init();
	// 定时器100毫秒的指示灯状态刷新（优先级低11|11）
	//tim3_init(999, 7199, 0xF);
	// 串口1控制台初始化
	u1_init();
	u1_printf("\r\n\033[31;4m欢迎使用stm32f103c8t6华慧物联网IOT控制终端 Design By 猫咪也有理想\033[0m\r\n");
	u1_printf("当前设备型号:%s,机器码:%s,客户id:%u\r\n", iot_data->model, iot_data->machineid, iot_data->customerid);
	if (check_init() == 0) {
		while (1);
	}
	// 按键中断初始化
	exti_init();
	// RTC时钟初始化
	init_rtc(0);
	// esp8266初始化
	esp8266_init();
	// 连接wifi
	iot_connect_wifi(iot_data->ssid, iot_data->password);
	// 定时器每隔3秒轮询服务器并更新IO状态数据(优先级最低11|00)
	tim2_init(29999, 7199, 0xC);
}


void iot_run(void) {
	while (1);
}


void TIM3_IRQHandler() {
	if (TIM3->SR & TIM_SR_UIF) {
		switch (g_net_status) {
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


void TIM2_IRQHandler() {
	u8 op_code;
	char buf[PACKAGE_DATA_SIZE];
	if ((TIM2->SR & TIM_SR_UIF) == RESET) {
		return;
	}
	TIM2->SR &= ~TIM_SR_UIF;
	if (g_net_status != WIFI_Connected) {
		return;
	}
	if (g_wait_sync == 1) {
		build_setstatus_package(buf, (u8)g_io_status, PACKAGE_DATA_SIZE);
	} else {
		build_getstatus_package(buf, PACKAGE_DATA_SIZE);
	}
	if (send_message(buf, PACKAGE_DATA_SIZE) != 0) {
		return;
	}
	if (strlen(buf) >= PACKAGE_DATA_SIZE) {
		return;
	}
	if (strstr(buf, "+IPD")) {
		op_code = process_message(buf);
		if (op_code == 1) {
			u1_printf("get io_status ok\r\n");
		} else if (op_code == 2) {
			u1_printf("set io_status ok\r\n");
			g_wait_sync = 0;
			force_update_status(-1);
		}
	} else {
		u1_printf("error:\r\n\033[35m%s\033[0m\r\n", buf);
	}
}


u8 iot_connect_wifi(const char* ssid, const char* passwd) {
	char buf[256];
	u8 times;
	if (g_net_status == WIFI_Connecting || g_net_status == WIFI_Connected) {
		return 1;
	}
	g_net_status = WIFI_Connecting;
	times = 1;
	do {
		u1_printf("设置esp8266为STA模式 \r\n");
		if (esp8266_send_cmd("AT+CWMODE=1", "OK", 200, NULL, 0)) {
			g_net_status = WIFI_NoInit;
			continue;
		}
		u1_printf("开始第[%u]次连接无线网络[%s] \r\n", times, ssid);
		snprintf(buf, 64, "AT+CWJAP=\"%s\",\"%s\"", ssid, passwd);
		if (esp8266_send_cmd(buf, "OK", 2000, NULL, 0)) {
			if (times++ > MAX_CONNECT_TIMES) {
				u1_printf("网络连接失败,请手动检查网络\r\n");
				g_net_status = WIFI_NoInit;
				return 1;
			}
			continue;
		}
		u1_printf("设置esp8266为非透传模式 \r\n");
		if (esp8266_send_cmd("AT+CIPMODE=0", "OK", 200, NULL, 0)) {
			continue;
		}
		u1_printf("设置esp8266为单播模式 \r\n");
		if (esp8266_send_cmd("AT+CIPMUX=0", "OK", 200, NULL, 0)) {
			continue;
		}
		u1_printf("查询esp8266网络状态 \r\n");
		memset(buf, 0, 256);
		if (esp8266_send_cmd("AT+CIPSTA?", "OK", 200, buf, 256)) {
			continue;
		}
		u1_printf("@@@:%s\r\n", buf);
		break;
	} while (1);
	g_net_status = WIFI_Connected;
	u1_printf("\r\n\033[36;1mDone\033[0m\r\n");
	return 0;
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


static void build_getstatus_package(char* buf, u16 buf_size) {
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


static void build_setstatus_package(char* buf, u8 new_status, u16 buf_size) {
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


static u8 send_message(char* databuf, u16 databuf_size) {
	char temp[64];
	char post_data[PACKAGE_DATA_SIZE];
	char output[PACKAGE_DATA_SIZE];
	u16 len;
	if (g_net_status != WIFI_Connected) {
		return 1;
	}
	snprintf(post_data, PACKAGE_DATA_SIZE,
		"POST / HTTP/1.1\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"User-Agent: iot_esp8266_module\r\n"
		"Content-Length:%d\r\n"
		"\r\n%s", strlen(databuf), databuf);
	snprintf(temp, 64, "AT+CIPSTART=\"TCP\",\"%s\",%d", iot_data->server_ip, iot_data->server_port);
	if (esp8266_send_cmd(temp, "CONNECT", 2000, NULL, 0) != 0) {
		return 2;
	}
	u1_printf("连接服务器[%s:%u]成功 \r\n", iot_data->server_ip, iot_data->server_port);

	len = strlen(post_data);
	snprintf(temp, 64, "AT+CIPSEND=%d", len);
	esp8266_send_cmd(temp, "OK", 200, NULL, 0);

	u1_printf("\033[0msendto >>>>>>\r\n\033[32m%s\033[0m \r\n", post_data);
	memset(output, 0, PACKAGE_DATA_SIZE);
	if (esp8266_send_cmd(post_data, "+IPD", 2000, output, PACKAGE_DATA_SIZE) == 0) {
		len = strlen(output);
		if (len < databuf_size - 1) {
			u1_printf("recvfrom <<<<<<\r\n\033[34m%s\033[0m \r\n", output);
			memset(databuf, 0, databuf_size);
			strncpy(databuf, output, len);
		}
	}
	if (esp8266_send_cmd("AT+CIPCLOSE", "OK", 200, NULL, 0) == 0) {
		IWDG->KR = 0xAAAA;    // 喂狗
		u1_printf("\r\n\033[0m通信结束,断开与服务器的连接 \r\n");
	}
	return 0;
}


static u8 check_init(void) {
	u8 isok;
	isok = 1;
	g_net_status = WIFI_NoInit;
	if ((u8)iot_data->model[0] == 0xFF) {
		u1_printf("need setmodel\r\n");
		isok = 0;
	}
	if ((u8)iot_data->ssid[0] == 0xFF) {
		u1_printf("need setssid\r\n");
		isok = 0;
	}
	if ((u8)iot_data->password[0] == 0xFF) {
		u1_printf("need setpassword\r\n");
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
