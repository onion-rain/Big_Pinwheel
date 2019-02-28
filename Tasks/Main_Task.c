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
#include "Global_Variable.h"

void MainTask(void const * argument)
{
  for(;;)
	{
		TimerTicks = xTaskGetTickCount();//��õ�ǰϵͳ����
		Remote_Handle();//ң�������
		osDelay(1);
	}
}

