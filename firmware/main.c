#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f10x.h>
#include "sys.h"
#include "cmd.h"

int main()
{
	//char buf[1024] = {0};
	/*设置优先级分组，2抢占，2响应*/
	NVIC_SetPriorityGrouping(2);
	exti_init();
	led_init();
	u1_init();
	u2_init();
	// 2秒双闪运行提示灯
	tim3_init(9999, 7199);
	
	u1_printf("\r\n\033[36;4m%s\033[0m\r\n", iot_data->welcome);
	u1_printf("当前机器码:%s\r\n", iot_data->machineid);
	/*
	esp8266_init();
	*/
	while (1) {
		delay(500);
		//esp8266_send("aa", buf);
		//u1_printf(buf);
	}
}
