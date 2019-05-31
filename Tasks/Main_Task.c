/** 
* @brief    主任务
* @details  This is the detail description.
* @author   Onion rain
* @date     2018.10.16
* @version  
* @par Copyright (c):
*       Onion rain
* @par 日志
*/

#include "Main_Task.h"
#include "cmsis_os.h"
#include "task.h"
#include "My_Remote.h"
#include "can.h"
#include "Can_Driver.hpp"
#include "Global_Variable.h"
#include "Dbus_Uart.h"

extern int16_t arm_flash, last_arm_flash, arm_flashed;

void MainTask(void const * argument)
{
  for(;;)
	{
		TimerTicks = xTaskGetTickCount();//获得当前系统心跳
		Remote_Handle();//遥控器句柄
		osDelay(1);
	}
}

