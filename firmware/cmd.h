#ifndef __CMD_H__
#define __CMD_H__

#include <stm32f10x.h>

void dispatch_cmdline(const char* cmdline);
u8 process_message(const char* message);

#define MODEl_MAX		20
#define MACHINEID_MAX	12
#define SSID_MAX		16
#define PASSWORD_MAX	16
#define SERVER_IP_MAX	20
#define SECRET_KEY_MAX	36

typedef struct
{
	char model[MODEl_MAX];
	char machineid[MACHINEID_MAX];
	char ssid[SSID_MAX];
	char password[PASSWORD_MAX];
	char server_ip[SERVER_IP_MAX];
	char secret_key[SECRET_KEY_MAX];
	unsigned short server_port;
	unsigned int customerid;
} IOT_TypeDef;


#define USER_DATA_BASE		0x0800A000
#define iot_data			((IOT_TypeDef *) USER_DATA_BASE)

#endif
