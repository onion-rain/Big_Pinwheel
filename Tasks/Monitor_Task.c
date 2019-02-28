/** 
* @brief    �������
* @details  This is the detail description.
* @author   Onion rain
* @date     2018.10.16
* @version  
* @par Copyright (c):
*       Onion rain
* @par ��־
*/  

#include "Monitor_Task.h"
#include "Global_Variable.h"
#include "music.h"
#include "My_RGB.h"
#include "My_Car.hpp"

static TickType_t MonitorTick;
//extern osThreadId MainTaskHandle;//������������Ҫ�ر����߳�

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
//		myprintf((char*)"������           ����״̬   ���ȼ�     ʣ��ջ   �������\r\n");
//		osThreadList((uint8_t *)&Buffer);
//		myprintf((char*)"%s\r\n", Buffer);
//    /*
//		#define tskBLOCKED_CHAR          ('B')  ��������
//		#define tskREADY_CHAR            ('R')  �������
//		#define tskDELETED_CHAR          ('D')  ����ɾ��
//		#define tskSUSPENDED_CHAR        ('S')  �������
//    */
		MonitorTick = xTaskGetTickCount();//��ȡ��ǰʱ��
		/*DBUSʧ�����*/
		if(Connection_monitor(DbusTick,50))//&& ChassisMode != SAFE_MODE)
		{
			DbusOutofContact = 1;
			Music_Play(warning);
			RGB_Rand();
		}
//		/*Ħ���ֱ�����ʧ�����*/
//		else if(Connection_monitor(Friction_l.LastUpdateTime,25))// || Connection_monitor(Friction_r.LastUpdateTime,25))
//		{
//			FrictionEncoderOutofContact = 1;
//			Music_Play(warning);
//			RGB_Rand();
//		}
		/*���ʧ�����*/
		else if(~(manager::CAN1_OnlineID | 0xffffff8f))//�е��������
		{
			Music_Play(warning);
			RGB_Rand();
		}
		/*���������ʧ�����*/
		else if(Connection_monitor(GyroTick,200))
		{
			GyroOutofContact = 1;
			RGB_Rand();
//	    Music_Play(Little_star);
	    Music_Play(warning);
		}
//		/*����ϵͳʧ�����*/
//		else if(Connection_monitor(JudgementTick,50))
//		{ 
//			JudgementOutofContact = 1;
//			RGB_Rand();
//		}
//		/*�Ӿ�����ʧ�����*/
//		else if(Connection_monitor(VisionTick,1500))
//		{
//			VisionOutofContact = 1;
//			RGB_Rand();
//		}
//		else if((hcan1.Instance->IER==0 || hcan2.Instance->IER == 0))//can�ж�ʹ��ʧ��
//		{
//			osThreadTerminate(MainTaskHandle);//ɾ��������
//			can_send_msg(&hcan1,0x200,0);
//			can_send_msg(&hcan1,0x1ff,0);
//			can_send_msg(&hcan2,0x200,0);
//			can_send_msg(&hcan2,0x1ff,0);
//			RGB_Rand();
//		}
		else
		{
			RGB_Gradational();//һ������
			DbusOutofContact = 0;
//			FrictionEncoderOutofContact = 0;
			GyroOutofContact = 0;
			JudgementOutofContact = 0;
			VisionOutofContact = 0;
		}
		osDelay(10);
	}
}



