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
#include "My_RGB.h"
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
		if(Connection_monitor(DbusTick,50))//&& ChassisMode != SAFE_MODE)
		{
			DbusOutofContact = 1;
			Music_Play(warning);
			RGB_Rand();
		}
//		/*摩擦轮编码器失联检测*/
//		else if(Connection_monitor(Friction_l.LastUpdateTime,25))// || Connection_monitor(Friction_r.LastUpdateTime,25))
//		{
//			FrictionEncoderOutofContact = 1;
//			Music_Play(warning);
//			RGB_Rand();
//		}
		/*电机失联检测*/
		else if(~(manager::CAN1_OnlineID | 0xffffff8f))//有电机不在线
		{
			Music_Play(warning);
			RGB_Rand();
		}
		/*凋敝陀螺仪失联检测*/
		else if(Connection_monitor(GyroTick,200))
		{
			GyroOutofContact = 1;
			RGB_Rand();
//	    Music_Play(Little_star);
	    Music_Play(warning);
		}
//		/*裁判系统失联检测*/
//		else if(Connection_monitor(JudgementTick,50))
//		{ 
//			JudgementOutofContact = 1;
//			RGB_Rand();
//		}
//		/*视觉串口失联检测*/
//		else if(Connection_monitor(VisionTick,1500))
//		{
//			VisionOutofContact = 1;
//			RGB_Rand();
//		}
//		else if((hcan1.Instance->IER==0 || hcan2.Instance->IER == 0))//can中断使能失败
//		{
//			osThreadTerminate(MainTaskHandle);//删除主任务
//			can_send_msg(&hcan1,0x200,0);
//			can_send_msg(&hcan1,0x1ff,0);
//			can_send_msg(&hcan2,0x200,0);
//			can_send_msg(&hcan2,0x1ff,0);
//			RGB_Rand();
//		}
		else
		{
			RGB_Gradational();//一切正常
			DbusOutofContact = 0;
//			FrictionEncoderOutofContact = 0;
			GyroOutofContact = 0;
			JudgementOutofContact = 0;
			VisionOutofContact = 0;
		}
		osDelay(10);
	}
}



