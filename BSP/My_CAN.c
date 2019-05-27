/** 
	* @brief    CAN
	* @details  CAN回调函数
	* @author   Onion rain
	* @date     2018.10.15 
	* @version  1.0
	* @par Copyright (c):  Onion rain
	* @par 日志
*/

#include "My_CAN.h"
#include "can.h"
#include "Global_Variable.h"
#include "Car_Driver.hpp"
#include "Dbus_Uart.h"
#include "My_SMDLED.h"

CAN_RxHeaderTypeDef RxHead;
extern int16_t arm_flash, last_arm_flash, arm_flashed;
extern uint8_t secondary_finished_flag;

static uint8_t data[8];//接收数据缓冲区

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&RxHead,data);
	#ifdef MASTER_CONTROL
		if(RxHead.StdId == 0x201)
			secondary_finished_flag = data[1];
	#endif
	#ifdef SECONDARY_CONTROL
	if(RxHead.StdId == 0x102)
	{
		RC_Ctl.rc.s1 = (data[0]<<8 | data[1])/10;
		RC_Ctl.rc.s2 = (data[0]<<8 | data[1])%10;
		arm_flash = data[2]<<8 | data[3];
		last_arm_flash = data[4]<<8 | data[5];
		arm_flashed = data[6]<<8 | data[7];
	}
	else
		manager::CANUpdate(hcan,&RxHead,data);
	#endif
}

