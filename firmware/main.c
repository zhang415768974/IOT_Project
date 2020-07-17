#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f10x.h>
#include "sys.h"
#include "cmd.h"

int main()
{
	char buf[128];
	/*设置优先级分组，2抢占，2响应*/
	NVIC_SetPriorityGrouping(2);
	exti_init();
	led_init();
	u1_init();
	u2_init();
	// 2秒双闪运行提示灯
	//tim3_init(9999, 7199);
	// 5秒一次心跳包请求状态
	tim4_init(49999, 7199);
	u1_printf("\r\n\033[36;4m%s\033[0m\r\n", iot_data->welcome);
	u1_printf("当前机器码:%s,客户id:%u\r\n", iot_data->machineid, iot_data->customerid);
	esp8266_init();
	while (1) {
		delay(500);
		memset(buf, 0, 128);
		iot_update(buf);
		if (strstr(buf, "+IPD")) {
			process_message(buf);
		} else {
			u1_printf("server error\r\n");
		}
	}
}
