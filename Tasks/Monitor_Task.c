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
#include "Dbus_Uart.h"
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



