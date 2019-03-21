/** 
* @brief    遥控器各种模式总控
* @details  This is the detail description. 
* @author   Onion rain
* @date     date 2018.11.9
* @version  1.0
* @par Copyright (c):  Onion rain
* @par 日志
*/

#include "My_Remote.h"
#include "My_Car.hpp"
#include "Dbus_Uart.h"
#include "can.h"
#include "define_all.h"
#include "Global_Variable.h"
#include "My_SMDLED.h"
#include "string.h"
#include <stdlib.h>

#define STARTING	0//<启动时参数
#define RUNNING		1//<运行时参数
#define ENDING		2//<结束时参数

static uint8_t remote_mode=0;
static uint8_t last_mode= 0xFF; //上一次遥控器的值，用于对比切换模式

extern int8_t RGB_Start_index[5][5];//声明于My)SMDLED.c，切换模式时清零防止不同模式间干扰
extern uint8_t RGB_Tail_num[5][5];//声明于My)SMDLED.c，切换模式时清零防止不同模式间干扰

uint8_t return_data = 0;//debug专属
uint8_t arm_flash = 0x00;//can_buffer[1]后8位，模式，每一位代表一个臂，1代表当前臂需要刷新，0表示保持现状，全0表示全灭
static uint8_t arm_flashed = 0x00;//已刷新过的臂

static void Rand_Purity_Color(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, rand()%6+1);
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, rand()%6+1);
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, rand()%6+1);
			SMD_LED_PWM_Init();
			break;
		case RUNNING:
			break;
		case ENDING:
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, 0);
			SMD_LED_PWM_Init();
			break;
	}
}
static void Rand_Color(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, RAND);
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, RAND);
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, RAND);
			SMD_LED_PWM_Init();
			break;
		case RUNNING:
			break;
		case ENDING:
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, 0);
			SMD_LED_PWM_Init();
			break;
	}
}
static void Sliding_Window(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%100 == 0)
			{
				return_data = SMD_LED_Running_Water_Effect_Configuration(0, SLIDING_WINDOW, 10, RED);
				return_data = SMD_LED_Running_Water_Effect_Configuration(1, SLIDING_WINDOW, 10, RED);
				return_data = SMD_LED_Running_Water_Effect_Configuration(2, SLIDING_WINDOW, 10, RED);
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, 0);
			SMD_LED_PWM_Init();
			break;
	}
}
static void Conveyer_Belt(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%200 == 0)
			{
				return_data = SMD_LED_Running_Water_Effect_Configuration(0, CONVEYER_BELT, 3, BLUE);
				return_data = SMD_LED_Running_Water_Effect_Configuration(1, CONVEYER_BELT, 3, BLUE);
				return_data = SMD_LED_Running_Water_Effect_Configuration(2, CONVEYER_BELT, 3, BLUE);
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, 0);
			SMD_LED_PWM_Init();
			break;
	}
}
static void run(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			#ifndef AUXILIARY
				if(HAL_GetTick()%3000 == 0)//设置需要刷新的臂
				{
					if(arm_flashed == 0x1f)//已全部被刷新过则清零
					{
						arm_flash = 0x00;
						arm_flashed = 0x00;
					}else
					{
						do arm_flash = 0x01<<rand()%5;
						while((arm_flash&arm_flashed) != 0x00);//目标臂已经被刷新过
						arm_flashed |= arm_flash;//更新已被刷新过的臂
					}
				}
			#endif
			if(HAL_GetTick()%100 == 0)
			{
				if(arm_flash == 0x00)//清零指令
				{
					memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
					return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, 0);
					return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
					return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, 0);
				}else
				{
					#ifndef AUXILIARY//主控
						if((arm_flash>>0)&0x01)//判断是否要刷新
							return_data = SMD_LED_Running_Water_Effect_Configuration(0, CONVEYER_BELT, 3, BLUE);
						if((arm_flash>>1)&0x01)
							return_data = SMD_LED_Running_Water_Effect_Configuration(1, CONVEYER_BELT, 3, BLUE);
						if((arm_flash>>2)&0x01)
							return_data = SMD_LED_Running_Water_Effect_Configuration(2, CONVEYER_BELT, 3, BLUE);
					#else//副控
						if((arm_flash>>3)&0x01)//判断是否要刷新
							return_data = SMD_LED_Running_Water_Effect_Configuration(0, CONVEYER_BELT, 3, BLUE);
						if((arm_flash>>4)&0x01)
							return_data = SMD_LED_Running_Water_Effect_Configuration(1, CONVEYER_BELT, 3, BLUE);
						if((arm_flash>>5)&0x01)
							return_data = SMD_LED_Running_Water_Effect_Configuration(2, CONVEYER_BELT, 3, BLUE);
					#endif
				}
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, 0);
			SMD_LED_PWM_Init();
			break;
	}
}
static void Progress_Bar_0(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%100 == 0)
			{
				return_data = SMD_LED_Running_Water_Effect_Configuration(0, PROGRESS_BAR_0, 0, RED);
				return_data = SMD_LED_Running_Water_Effect_Configuration(1, PROGRESS_BAR_0, 0, RED);
				return_data = SMD_LED_Running_Water_Effect_Configuration(2, PROGRESS_BAR_0, 0, RED);
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, 0);
			SMD_LED_PWM_Init();
			break;
	}
}
static void Progress_Bar_1(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%100 == 0)
			{
				return_data = SMD_LED_Running_Water_Effect_Configuration(0, PROGRESS_BAR_1, 0, BLUE);
				return_data = SMD_LED_Running_Water_Effect_Configuration(1, PROGRESS_BAR_1, 0, BLUE);
				return_data = SMD_LED_Running_Water_Effect_Configuration(2, PROGRESS_BAR_1, 0, BLUE);
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, 0);
			SMD_LED_PWM_Init();
			break;
	}
}
static void Progress_Bar_2(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%85 == 0)
			{
				return_data = SMD_LED_Running_Water_Effect_Configuration(0, PROGRESS_BAR_2, 3, BLUE);
				return_data = SMD_LED_Running_Water_Effect_Configuration(1, PROGRESS_BAR_2, 3, BLUE);
				return_data = SMD_LED_Running_Water_Effect_Configuration(2, PROGRESS_BAR_2, 3, BLUE);
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			memset(RGB_Tail_num, 0x00, sizeof(RGB_Tail_num));
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, 0);
			SMD_LED_PWM_Init();
			break;
	}
}
static void Tetris(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%100 == 0)
			{
				return_data = SMD_LED_Running_Water_Effect_Configuration(0, TETRIS, 10, RAND);
				return_data = SMD_LED_Running_Water_Effect_Configuration(1, TETRIS, 10, RAND);
				return_data = SMD_LED_Running_Water_Effect_Configuration(2, TETRIS, 10, RAND);
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, 0);
			SMD_LED_PWM_Init();
			break;
	}
}
/** 
    * @brief 安全模式
*/
static void safe_mode(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, 0);
			SMD_LED_PWM_Init();
			break;
		case RUNNING:
			break;
		case ENDING:
			break;
	}
}
/** 
* @brief  遥控器mode分发
* @param [in]  mode 遥控器s1*10+s2
* @param [in]  type 运行参数
* @par 日志
*
*/
static void Remote_Distribute(uint8_t mode, uint8_t type)
{
	switch(mode)
	{
		case 22:safe_mode(type);break;//安全模式
		case 21:Rand_Color(type);break;	//随机色
		case 13:Tetris(type);break;	//俄罗斯方块
//		case 33:Rand_Purity_Color(type);break;//随机纯色
		case 33:Conveyer_Belt(type);break;//传送带
		case 12:Sliding_Window(type);break;//滑动窗口
		case 32:run(type);break;//大符运行模式
		case 11:Progress_Bar_0(type);break;//交叉进度条
		case 31:Progress_Bar_1(type);break;//同向进度条
		case 23:Progress_Bar_2(type);break;//滴水进度条
		default:break;
	}
	manager::CANSend();
}

//标准操作函数和具体功能无关
void Remote_Handle(void)
{
	remote_mode=RC_Ctl.rc.s1*10+RC_Ctl.rc.s2;   //从遥控器获取当前模式
	if(remote_mode!=last_mode)                   //如果当前模式和之前的模式不一样的话 
	{
		Remote_Distribute(last_mode,ENDING);       //退出之前的模式
		Remote_Distribute(remote_mode,STARTING);   //启用当前模式 开始部分
//		Remote_Distribute(remote_mode,RUNNING);    //跑一次当前模式的running
		last_mode = remote_mode;
	}
	Remote_Distribute(remote_mode,RUNNING);      //持续当前模式
}



