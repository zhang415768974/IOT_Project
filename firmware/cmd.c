#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "core.h"
#include "sys.h"
#include "usart.h"
#include "rtc.h"
#include "stmflash.h"
#include "esp8266.h"
#include "md5.h"


#define	MAX_PARAM_NUM 4
#define	MAX_PARAM_SIZE 64
#define	CMD_BUF_SIZE (MAX_PARAM_NUM * MAX_PARAM_SIZE)
#define MAX_RESPONSE_SIZE 128
#define SIGNATURE_BUF_SIZE 64


static void reset_system() {
	u8 count = 4;
	while (count--) {
		u1_printf("\r\b系统将在\033[1;31m%d\033[0m秒后重启...", count);
		delay_ms(1000);
	}
	SCB->AIRCR = 0x05FA0000 | SCB_AIRCR_SYSRESETREQ_Msk;
}


static void set_gpio(char (*argv)[MAX_PARAM_SIZE]) {
	int idx = atoi(argv[1]);
	if (idx < 0 || idx > 7) {
		return;
	}
	// 有状态等待同步,挂起io操作
	if (g_wait_sync == 1) {
		return;
	}
	if (atoi(argv[2])) {
		g_io_status |= 1 << idx;
	} else {
		g_io_status &= ~(1 << idx);
	}
	force_update_status(g_io_status);
}


static void set_machineid(const char* str) {
	IOT_TypeDef iot;
	if (strlen(str) != 8) {
		u1_printf("机器码长度必须是8个字符串\r\n");
		return;
	}
	memcpy(&iot, (const void*)USER_DATA_BASE, sizeof(iot));
	memset(iot.machineid, 0, MACHINEID_MAX);
	strncpy(iot.machineid, str, strlen(str));
	flash_write(USER_DATA_BASE, (char*)&iot, sizeof(iot));
}


static void set_model(const char* str) {
	IOT_TypeDef iot;
	memcpy(&iot, (const void*)USER_DATA_BASE, sizeof(iot));
	memset(iot.model, 0, MODEl_MAX);
	strncpy(iot.model, str, strlen(str));
	flash_write(USER_DATA_BASE, (char*)&iot, sizeof(iot));
}


static void set_ssid(const char* str) {
	IOT_TypeDef iot;
	memcpy(&iot, (const void*)USER_DATA_BASE, sizeof(iot));
	memset(iot.ssid, 0, SSID_MAX);
	strncpy(iot.ssid, str, strlen(str));
	flash_write(USER_DATA_BASE, (char*)&iot, sizeof(iot));
}


static void set_password(const char* str) {
	IOT_TypeDef iot;
	memcpy(&iot, (const void*)USER_DATA_BASE, sizeof(iot));
	memset(iot.password, 0, PASSWORD_MAX);
	strncpy(iot.password, str, strlen(str));
	flash_write(USER_DATA_BASE, (char*)&iot, sizeof(iot));
}


static void set_serverip(const char* str) {
	IOT_TypeDef iot;
	memcpy(&iot, (const void*)USER_DATA_BASE, sizeof(iot));
	memset(iot.server_ip, 0, SERVER_IP_MAX);
	strncpy((char*)iot.server_ip, str, strlen(str));
	flash_write(USER_DATA_BASE, (char*)&iot, sizeof(iot));
}


static void set_secretkey(const char* str) {
	IOT_TypeDef iot;
	memcpy(&iot, (const void*)USER_DATA_BASE, sizeof(iot));
	memset(iot.secret_key, 0, SECRET_KEY_MAX);
	strncpy((char*)iot.secret_key, str, strlen(str));
	flash_write(USER_DATA_BASE, (char*)&iot, sizeof(iot));
}


static void set_serverport(u16 port) {
	IOT_TypeDef iot;
	memcpy(&iot, (const void*)USER_DATA_BASE, sizeof(iot));
	iot.server_port = port;
	flash_write(USER_DATA_BASE, (char*)&iot, sizeof(iot));
}


static void set_customerid(u32 customerid) {
	IOT_TypeDef iot;
	memcpy(&iot, (const void*)USER_DATA_BASE, sizeof(iot));
	iot.customerid = customerid;
	flash_write(USER_DATA_BASE, (char*)&iot, sizeof(iot));
}


static void md5sum(const char* message) {
	char temp[MD5_HASHSIZE * 2 + 1];
	memset(temp, 0, sizeof(temp));
	md5_hexdigest(message, strlen(message), temp);
	u1_printf("%s\r\n", temp);
}


void dispatch_cmdline(const char* cmdline) {
	int i;
	char buf[CMD_BUF_SIZE];
	char result[256];
	char argv[MAX_PARAM_NUM][MAX_PARAM_SIZE];
	char *p, *out_ptr;
	const char* cmd;

	strncpy(buf, cmdline, strlen(cmdline) + 1);
	memset(argv, 0, CMD_BUF_SIZE);
	p = strtok_r(buf, " ", &out_ptr);
	strncpy(argv[0], p, strlen(p));
	for (i = 0; i != MAX_PARAM_NUM && p; ++i) {
		strncpy(argv[i], p, strlen(p));
		p = strtok_r(NULL, " ", &out_ptr);
	}
	cmd = argv[0];

	if (strcmp(cmd, "reset") == 0) {
		reset_system();
	} else if (strcmp(cmd, "gpio") == 0) {
		set_gpio(argv);
	} else if (strcmp(cmd, "setmid") == 0) {
		set_machineid(argv[1]);
	} else if (strcmp(cmd, "atcmd") == 0) {
		sprintf(buf, "%s", argv[1]);
		memset(result, 0, 256);
		esp8266_send_cmd(buf, "OK", 200, result, 256);
		u1_printf("%s\r\n", result);
	} else if (strcmp(cmd, "md5sum") == 0) {
		md5sum(argv[1]);
	} else if (strcmp(cmd, "sleep") == 0) {
		delay_ms(atoi(argv[1]));
	} else if (strcmp(cmd, "setmodel") == 0) {
		set_model(argv[1]);
	} else if (strcmp(cmd, "setssid") == 0) {
		set_ssid(argv[1]);
	} else if (strcmp(cmd, "setpwd") == 0) {
		set_password(argv[1]);
	} else if (strcmp(cmd, "setip") == 0) {
		set_serverip(argv[1]);
	} else if (strcmp(cmd, "setport") == 0) {
		set_serverport(atoi(argv[1]));
	} else if (strcmp(cmd, "setcid") == 0) {
		set_customerid(strtoul(argv[1], NULL, 0));
	} else if (strcmp(cmd, "setkey") == 0) {
		set_secretkey(argv[1]);
	} else if (strcmp(cmd, "settimestamp") == 0) {
		init_rtc(atoi(argv[1]));
	} else if (strcmp(cmd, "stop") == 0) {
		TIM2->CR1 &= ~TIM_CR1_CEN;
	} else if (strcmp(cmd, "start") == 0) {
		TIM2->CR1 |= TIM_CR1_CEN;
	} else if (strcmp(cmd, "test") == 0) {
		memset(result, 0, 256);
		esp8266_send_cmd("AT+CIPSTART=\"TCP\",\"www.hhdz1234.com\",80", "OK", 200, result, 256);
		u1_printf("%s\r\n", result);
		memset(result, 0, 256);
		esp8266_send_cmd("AT+CIPMODE=1", "OK", 200, result, 256);
		u1_printf("%s\r\n", result);
		esp8266_send_cmd("AT+CIPSEND", ">", 200, NULL, 0);
		memset(result, 0, 256);
		esp8266_send_cmd("GET http://www.hhdz1234.com/dh/ABC.asp\r\n", "+IPD", 200, result, 256);
		u1_printf("%s\r\n", result);
		esp8266_send_cmd("+++", NULL, 200, NULL, 0);
		esp8266_send_cmd("AT+CIPCLOSE", NULL, 200, NULL, 0);
	}
}


static unsigned char oauth(const char* data) {
	u16 len, offset;
	char signature_src[SIGNATURE_BUF_SIZE];
	char temp[MD5_HASHSIZE * 2 + 1];
	len = strlen(data);
	if (data && len > 33 && len < SIGNATURE_BUF_SIZE) {
		if (data[0] == 'r' && data[1] == 'e' && data[2] == 's') {
			memset(signature_src, 0, SIGNATURE_BUF_SIZE);
			offset = len - 32;
			strncpy(signature_src, data, offset);
			strncat(signature_src, iot_data->secret_key, SIGNATURE_BUF_SIZE);
			memset(temp, 0, sizeof(temp));
			md5_hexdigest(signature_src, strlen(signature_src), temp);
			return strncmp(&data[offset], temp, 32) == 0;
		}
	}
	return 0;
}


u8 process_message(const char* message) {
	const char *result;
	char buf[MAX_RESPONSE_SIZE];
	u16 len;
	char *p, *out_ptr;
	u8 server_data, op_code;
	result = strstr(message, "res");
	// 必须严格验证从服务器收到的数据完整性
	if (oauth(result) == 0) {
		u1_printf("oauth error\r\n");
		return 0;
	}
	led_double(80);
	len = strlen(result);
	if (len >= MAX_RESPONSE_SIZE) {
		return 0;
	}
	memset(buf, 0, MAX_RESPONSE_SIZE);
	strncpy(buf, result, len);

	p = strtok_r(buf, "#", &out_ptr); // res
	p = strtok_r(NULL, "#", &out_ptr); // 返回码
	op_code = atoi(p);
	p = strtok_r(NULL, "#", &out_ptr); // io状态值
	server_data = atoi(p);
	if (g_wait_sync == 0 && server_data != g_io_status) {
		g_io_status = server_data;
		force_update_status(-1);
		u1_printf("io_status changed by server\r\n");
	}
	return op_code;
}
