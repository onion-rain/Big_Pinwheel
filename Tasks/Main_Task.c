/** 
* @brief    ������
* @details  This is the detail description.
* @author   Onion rain
* @date     2018.10.16
* @version  
* @par Copyright (c):
*       Onion rain
* @par ��־
*/

#include "Main_Task.h"
#include "cmsis_os.h"
#include "task.h"
#include "My_Remote.h"
#include "can.h"
#include "Can_Driver.hpp"
#include "Global_Variable.h"
#include "Dbus_Uart.h"

extern uint8_t arm_check, arm_flesh;

void MainTask(void const * argument)
{
  for(;;)
	{
		TimerTicks = xTaskGetTickCount();//��õ�ǰϵͳ����
		Remote_Handle();//ң�������
		can_buffer[0] = RC_Ctl.rc.s1*10+RC_Ctl.rc.s2;
		can_buffer[1] = arm_flesh;
		can_send_msg(&hcan1, 0x111, can_buffer);//�����ط���Ϣ
		osDelay(1);
	}
}

