#ifndef __CMD_H__
#define __CMD_H__


void process_cmdline(const char* cmdline);

#define WELCOME_MAX			128
#define MACHINEID_MAX		16
#define WIFI_NAME_MAX		32
#define WIFI_PASSWORD_MAX	32
#define SERVER_IP_MAX		32

typedef struct
{
	char welcome[WELCOME_MAX];
	char machineid[MACHINEID_MAX];
	char wifi_name[WIFI_NAME_MAX];
	char wifi_password[WIFI_PASSWORD_MAX];
	char server_ip[SERVER_IP_MAX];
	short server_port;
} IOT_TypeDef;


#define USER_DATA_BASE		0x0800A000
#define iot_data			((IOT_TypeDef *) USER_DATA_BASE)

#endif
