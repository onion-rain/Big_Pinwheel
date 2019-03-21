/** 
* @brief    大符
* @details  
* @author   Onion rain
* @date     2019.3.21
* @version  1.0
* @par Copyright (c):  Onion rain
* @par 日志
*/

#include "My_Buff.h"
#include "My_SMDLED.h"
#include "define_all.h"
#include "string.h"
#include <stdlib.h>

extern int8_t RGB_Start_index[5][5];//声明于My)SMDLED.c，切换模式时清零防止不同模式间干扰
extern uint8_t RGB_Tail_num[5][5];//声明于My)SMDLED.c，切换模式时清零防止不同模式间干扰

static uint8_t return_data = 0;//debug专属
uint8_t arm_flash = 0x00;//can_buffer[1]后8位，模式，每一位代表一个臂，1代表当前臂需要刷新，0表示保持现状，全0表示全灭
uint8_t last_arm_flash = 0x00;//上次遍历过的臂
static uint8_t arm_flashed = 0x00;//已刷新过的臂

void buff_conveyer_belt(void)
{
	#ifndef AUXILIARY//主控
	if((arm_flash>>0)&0x01)//判断是否刷新传送带
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, CONVEYER_BELT, 3, BLUE);
		if((arm_flash>>1)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, CONVEYER_BELT, 3, BLUE);
		if((arm_flash>>2)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, CONVEYER_BELT, 3, BLUE);
	#else//副控
		if((arm_flash>>3)&0x01)//判断是否刷新传送带
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, CONVEYER_BELT, 3, BLUE);
		if((arm_flash>>4)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, CONVEYER_BELT, 3, BLUE);
		if((arm_flash>>5)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, CONVEYER_BELT, 3, BLUE);
	#endif
}

void buff_all_on(void)
{
	#ifndef AUXILIARY//主控
		if((last_arm_flash>>0)&0x01)//判断是否刷新全亮
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, BLUE);
		if((last_arm_flash>>1)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, BLUE);
		if((last_arm_flash>>2)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, BLUE);
	#else//副控
		if((last_arm_flash>>3)&0x01)//判断是否刷新全亮
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, BLUE);
		if((last_arm_flash>>4)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, BLUE);
		if((last_arm_flash>>5)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, BLUE);
	#endif
}

void buff_reset(void)
{
	arm_flash = 0x00;
	arm_flashed = 0x00;
	last_arm_flash = 0x00;
}

void buff_flash(void)
{
	if(arm_flash == 0x00)//清零指令
	{
		memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
		return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, 0);
		return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
		return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, 0);
	}else
	{
		buff_conveyer_belt();
		buff_all_on();
	}
	SMD_LED_PWM_Init();
}

void buff_new_armnum_produce(void)
{
	if(HAL_GetTick()%3000 == 0)//设置需要刷新的臂
	{
		if(arm_flashed == 0x1f)//已全部被刷新过则清零
		{
			arm_flash = 0x00;
			arm_flashed = 0x00;
			last_arm_flash = 0x00;
		}else
		{
			last_arm_flash = arm_flash;//保存上次更新的臂标号
			do arm_flash = 0x01<<rand()%5;//随机生成下一个目标臂标号
			while((arm_flash&arm_flashed) != 0x00);//目标臂已经被刷新过则重新生成
			arm_flashed |= arm_flash;//更新已被刷新过的臂
		}
	}
}
