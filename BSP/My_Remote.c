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

#define STARTING	0//<启动时参数
#define RUNNING		1//<运行时参数
#define ENDING		2//<结束时参数

static uint8_t remote_mode=0;
static uint8_t last_mode= 0xFF; //上一次遥控器的值，用于对比切换模式

/** 
    * @brief 大符运行模式
*/
static void run_mode(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			break;
		case ENDING:
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
			break;
		case RUNNING:
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
static void Remote_Distribute(uint8_t mode,uint8_t type)
{
	switch(mode)
	{
		case 22:safe_mode(type);break;//安全模式
		case 32:run_mode(type);break;	//运行模式
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
		Remote_Distribute(remote_mode,RUNNING);    //跑一次当前模式的running
		last_mode = remote_mode;
	}
	Remote_Distribute(remote_mode,RUNNING);      //持续当前模式
}



