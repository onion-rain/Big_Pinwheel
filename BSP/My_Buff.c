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
#include "can.h"
#include "Can_Driver.hpp"
#include "Global_Variable.h"
#include <stdlib.h>

#define BUFF_COLOR RED

extern int8_t RGB_Start_index[5][5];//声明于My)SMDLED.c，切换模式时清零防止不同模式间干扰
extern uint8_t RGB_Tail_num[5][5];//声明于My)SMDLED.c，切换模式时清零防止不同模式间干扰

static uint8_t return_data = 0;//debug专属
uint8_t arm_flash = 0x00;//can_buffer[1]后8位，模式，每一位代表一个臂，1代表当前臂需要刷新，0表示保持现状，全0表示全灭
uint8_t last_arm_flash = 0x00;//上次遍历过的臂
uint8_t arm_flashed = 0x00;//已刷新过的臂
TickType_t LastShootTick;

#ifdef AUXILIARY//副控
	uint8_t flag_auxiliary = 0;//副控专属，首次进入打符成功灯效模式标志
#endif

#ifndef AUXILIARY//主控
	uint16_t Unprogrammable_Light_Bar = 0x0000;//不可编程的灯条
	uint8_t hit[17] = {0};//记录五个装甲板的打击次数
	uint8_t auxiliary_finished_flag = 0;//副控完成打符成功灯效标志，1为已完成，在can回调函数中更新

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)//装甲板受到打击回调函数
{
	switch(GPIO_Pin)
	{
		case GPIO_PIN_6:
			if(HAL_GetTick()-LastShootTick > 100)
			{
				hit[0x10]++;
				LastShootTick = HAL_GetTick();
			}
			break;
		case GPIO_PIN_15:
			if(HAL_GetTick()-LastShootTick > 100)
			{
				hit[0x08]++;
				LastShootTick = HAL_GetTick();
			}
			break;
		case GPIO_PIN_14:
			if(HAL_GetTick()-LastShootTick > 100)
			{
				hit[0x04]++;
				LastShootTick = HAL_GetTick();
			}
			break;
		case GPIO_PIN_13:
			if(HAL_GetTick()-LastShootTick > 100)
			{
				hit[0x02]++;
				LastShootTick = HAL_GetTick();
			}
			break;
		case GPIO_PIN_12:
			if(HAL_GetTick()-LastShootTick > 100)
			{
				hit[0x01]++;
				LastShootTick = HAL_GetTick();
			}
			break;
	}
}
#endif

void clear_with_purity_color(uint8_t color)//以纯色填充
{
	SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, color);
	SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, color);
	SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, color);
}

void buff_conveyer_belt(void)//大符灯效
{
	#ifndef AUXILIARY//主控
	if((arm_flash>>0)&0x01)//判断是否刷新传送带
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>1)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>2)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, CONVEYER_BELT, 3, BUFF_COLOR);
	#else//副控
		if((arm_flash>>3)&0x01)//判断是否刷新传送带
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>4)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>5)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, CONVEYER_BELT, 3, BUFF_COLOR);
	#endif
}

void buff_all_on(void)//判断哪些臂已完成击打，设置为全亮
{
	#ifndef AUXILIARY//主控
		if((last_arm_flash>>0)&0x01)//判断是否刷新全亮
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>1)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>2)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, BUFF_COLOR);
	#else//副控
		if((last_arm_flash>>3)&0x01)//判断是否刷新全亮
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>4)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>5)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, BUFF_COLOR);
	#endif
}

uint8_t buff_sucess_process_var(void)//大符全部击打成功灯效
{
	SMD_LED_Running_Water_Effect_Configuration(0, PROGRESS_BAR_1, 4, BUFF_COLOR);
	SMD_LED_Running_Water_Effect_Configuration(1, PROGRESS_BAR_1, 4, BUFF_COLOR);
	return SMD_LED_Running_Water_Effect_Configuration(2, PROGRESS_BAR_1, 4, BUFF_COLOR);
}

void buff_reset(void)//大符初始化
{
	clear_with_purity_color(0);//整个大符清屏
	#ifndef AUXILIARY//主控
		hit[0] = 1;//开启第一个臂
		auxiliary_finished_flag = 0;//副控成功打符灯效完成标志清零
		Unprogrammable_Light_Bar = 0x0000;//不可编程灯条清屏
	#endif
	arm_flash = 0x00;
	arm_flashed = 0x00;
	last_arm_flash = 0x00;
}

void buff_flag_sucess(void)//打符成功标志位处理
{
	arm_flash = 0xff;
	arm_flashed = 0xff;
	last_arm_flash = 0xff;
}
#ifndef AUXILIARY//主控
void buff_new_armnum_produce(void)//设置需要刷新的臂
{
	if(arm_flashed == 0x1f)/*已全部被刷新过*/
	{
		memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));//起始index清零，准备成功打符灯效，副控使用标志flag_auxiliary判断是否为首次进行成功打符灯效判断是否要进行index清零
		buff_flag_sucess();
	}else//还有未被刷新的臂
	{
		last_arm_flash = arm_flash;//保存上次更新的臂标号
		do arm_flash = 0x01<<rand()%5;//随机生成下一个目标臂标号
		while((arm_flash&arm_flashed) != 0x00);//目标臂已经被刷新过则重新生成
		arm_flashed |= arm_flash;//更新已被刷新过的臂
		Unprogrammable_Light_Bar |= arm_flash<<5;//此刻刷新的臂装甲板灯条亮
	}
}
#endif
void buff_flash(void)//大符刷新函数，线程中周期调用
{
	#ifndef AUXILIARY//主控
		if(arm_flash!=0xff && hit[arm_flash])//正确装甲板被击打
		{
			Unprogrammable_Light_Bar |= arm_flash;//此刻arm_flash还未被刷新，被击打的臂臂灯条亮
			buff_new_armnum_produce();//设置需要刷新的臂
			memset(hit, 0, 17);//装甲板击打数据清零
		}
	#endif
	if(arm_flashed == 0xff && arm_flash == 0xff && last_arm_flash == 0xff)//打符成功
	{
		#ifdef AUXILIARY//副控
			if(flag_auxiliary == 0)//判断是否为首次进行成功打符灯效判断是否要进行index清零,准备成功打符灯效
				memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			flag_auxiliary = 1;//下次便不需要进行start_index清零
			if(buff_sucess_process_var()==0 && flag_auxiliary<=3)//副控进度条完成并且首次进行初始化(副控前两次初始化向主控发送进度条完成的消息，之后不再发送)
			{
				buff_reset();
				flag_auxiliary++;//副控专属，首次进入打符成功灯效模式标志
				can_buffer[0] = 1;
				can_send_msg(&hcan1, 0x222, can_buffer);//给主控发信息
			}
		#else //主控
		if(buff_sucess_process_var() == 0)//主控进度条完成，等待副控完成
			if(auxiliary_finished_flag == 1)//副控进度条完成，大符初始化
				buff_reset();
			#endif
	}else
	{
		#ifdef AUXILIARY//副控
			flag_auxiliary = 0;
		#endif
		buff_conveyer_belt();//根据标志位选择臂进行传送带灯效显示
		buff_all_on();//根据标志位选择臂进行全亮显示
	}
	SMD_LED_PWM_Init();
}

