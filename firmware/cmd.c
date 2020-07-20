#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f10x.h>

#include "sys.h"
#include "cmd.h"
#include "md5.h"

#define	MAX_PARAM_NUM 4
#define	MAX_PARAM_SIZE 64
#define	CMD_BUF_SIZE (MAX_PARAM_NUM * MAX_PARAM_SIZE)

#define MAX_RESPONSE_SIZE 128

u8 io_status;
extern u8 wait_sync;

static void reset_system() {
	unsigned char count = 4;
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
	if (wait_sync == 1) {
		return;
	}
	
	if (atoi(argv[2])) {
		io_status |= 1 << idx;
	} else {
		io_status &= ~(1 << idx);
	}
	force_update_status(io_status);
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


static void set_wifiname(const char* str) {
	IOT_TypeDef iot;
	memcpy(&iot, (const void*)USER_DATA_BASE, sizeof(iot));
	memset(iot.wifi_name, 0, WIFI_NAME_MAX);
	strncpy(iot.wifi_name, str, strlen(str));
	flash_write(USER_DATA_BASE, (char*)&iot, sizeof(iot));
}


static void set_wifipwd(const char* str) {
	IOT_TypeDef iot;
	memcpy(&iot, (const void*)USER_DATA_BASE, sizeof(iot));
	memset(iot.wifi_password, 0, WIFI_PASSWORD_MAX);
	strncpy(iot.wifi_password, str, strlen(str));
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


void process_cmdline(const char* cmdline) {
	int i;
	char buf[CMD_BUF_SIZE];
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

	if (strncmp(cmd, "reset", MAX_PARAM_SIZE) == 0) {
		reset_system();
	} else if (strncmp(cmd, "gpio", MAX_PARAM_SIZE) == 0) {
		set_gpio(argv);
	} else if (strncmp(cmd, "setmid", MAX_PARAM_SIZE) == 0) {
		set_machineid(argv[1]);
	} else if (strncmp(cmd, "atcmd", MAX_PARAM_SIZE) == 0) {
		sprintf(buf, "%s", argv[1]);
		esp8266_cmd(buf, "OK", 200, NULL);
	} else if (strncmp(cmd, "md5sum", MAX_PARAM_SIZE) == 0) {
		md5sum(argv[1]);
	} else if (strncmp(cmd, "sleep", MAX_PARAM_SIZE) == 0) {
		delay_ms(atoi(argv[1]));
	} else if (strcmp(cmd, "setmodel") == 0) {
		set_model(argv[1]);
	} else if (strncmp(cmd, "setwifiname", MAX_PARAM_SIZE) == 0) {
		set_wifiname(argv[1]);
	} else if (strncmp(cmd, "setwifipwd", MAX_PARAM_SIZE) == 0) {
		set_wifipwd(argv[1]);
	} else if (strncmp(cmd, "setserverip", MAX_PARAM_SIZE) == 0) {
		set_serverip(argv[1]);
	} else if (strncmp(cmd, "setserverport", MAX_PARAM_SIZE) == 0) {
		set_serverport(atoi(argv[1]));
	} else if (strncmp(cmd, "setcid", MAX_PARAM_SIZE) == 0) {
		set_customerid(strtoul(argv[1], NULL, 0));
	} else if (strncmp(cmd, "setkey", MAX_PARAM_SIZE) == 0) {
		set_secretkey(argv[1]);
	} else if (strncmp(cmd, "settimestamp", MAX_PARAM_SIZE) == 0) {
		init_rtc(atoi(argv[1]));
	}
}


static unsigned char oauth(const char* data) {
	u32 len, offset;
	char signature_src[SIGNATURE_BUF_SIZE];
	char temp[MD5_HASHSIZE * 2 + 1];
	len = strlen(data);
	if (data && len > 32 && len < SIGNATURE_BUF_SIZE) {
		if (data[0] == 'r' && data[1] == 'e' && data[2] == 's') {
			memset(signature_src, 0, SIGNATURE_BUF_SIZE);
			offset = len - 32;
			strncpy(signature_src, data, offset);
			strncat(signature_src, iot_data->secret_key, SIGNATURE_BUF_SIZE);
			memset(temp, 0, sizeof(temp));
			md5_hexdigest(signature_src, strlen(signature_src), temp);
			return strncmp(&data[offset], temp, 256) == 0;
		}
	}
	return 0;
}


u8 process_message(const char* message) {
	const char *result;
	char buf[MAX_RESPONSE_SIZE];
	u16 len;
	char *p, *out_ptr;
	u8 iodata_server, op_code;
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
	iodata_server = atoi(p);
	if (wait_sync == 0 && iodata_server != io_status) {
		io_status = iodata_server;
		u1_printf("io_status changed by server\r\n");
	}
	return op_code;
}
