/** 
* @brief    监控任务
* @details  This is the detail description.
* @author   Onion rain
* @date     2018.10.16
* @version  
* @par Copyright (c):
*       Onion rain
* @par 日志
*/  

#include "Monitor_Task.h"
#include "Global_Variable.h"
#include "music.h"
#include "Dbus_Uart.h"
#include "My_Car.hpp"

static TickType_t MonitorTick;
//extern osThreadId MainTaskHandle;//监控任务可能需要关闭主线程

static uint8_t Connection_monitor(TickType_t tick,TickType_t monitorperiod)
{
	return MonitorTick - tick > monitorperiod;
}

void MonitorTask(void const * argument)
{
//	uint8_t Buffer[500];
  for(;;)
	{
//		myprintf((char*)"=======================================================\r\n");
//		myprintf((char*)"任务名           任务状态   优先级     剩余栈   任务序号\r\n");
//		osThreadList((uint8_t *)&Buffer);
//		myprintf((char*)"%s\r\n", Buffer);
//    /*
//		#define tskBLOCKED_CHAR          ('B')  任务阻塞
//		#define tskREADY_CHAR            ('R')  任务就绪
//		#define tskDELETED_CHAR          ('D')  任务删除
//		#define tskSUSPENDED_CHAR        ('S')  任务挂起
//    */
		MonitorTick = xTaskGetTickCount();//获取当前时刻
		/*DBUS失联检测*/
		if(Connection_monitor(DbusTick,50))
		{
			DbusOutofContact = 1;
			RC_Ctl.rc.s1 = 4;
			RC_Ctl.rc.s2 = 4;
		}else
			DbusOutofContact = 0;
		osDelay(10);
	}
}



