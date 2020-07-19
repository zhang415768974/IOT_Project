#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f10x.h>
#include "sys.h"
#include "cmd.h"


int main()
{
	/*设置优先级分组，2抢占，2响应*/
	NVIC_SetPriorityGrouping(2);
	iwdg_init();
	// 定时器1秒一次看门狗喂狗,5秒不喂自动复位MCU
	tim3_init(9999, 7199);
	// 定时器100毫秒的指示灯状态刷新
	tim2_init(999, 7199);
	exti_init();
	led_init();
	u1_init();
	if (check_init() == RESET) {
		while (1);
	}
	u2_init();
	// 5秒一次心跳包请求状态
	tim4_init(49999, 7199);
	u1_printf("\r\n\033[31;4m欢迎使用stm32f103c8t6华慧物联网IOT控制终端 Design By 猫咪也有理想\033[0m\r\n");
	u1_printf("当前设备型号:%s,机器码:%s,客户id:%u\r\n", iot_data->model, iot_data->machineid, iot_data->customerid);
	esp8266_init();
	while (1);
}
