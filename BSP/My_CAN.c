/** 
	* @brief    CAN
	* @details  CAN�ص�����
	* @author   Onion rain
	* @date     2018.10.15 
	* @version  1.0
	* @par Copyright (c):  Onion rain
	* @par ��־
*/

#include "My_CAN.h"
#include "can.h"
#include "Global_Variable.h"
#include "Car_Driver.hpp"
#include "Dbus_Uart.h"
#include "My_SMDLED.h"
#include "My_Buff.h"

CAN_RxHeaderTypeDef RxHead;

#ifdef MASTER_CONTROL//����
	extern uint8_t secondary_finished_flag;
#endif

static uint8_t data[8];//�������ݻ�����

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&RxHead,data);
	#ifdef MASTER_CONTROL
		if(RxHead.StdId == 0x201)
			secondary_finished_flag = data[1];
	#endif
	#ifdef SECONDARY_CONTROL
		if(RxHead.StdId == 0x100)
		{
			RC_Ctl.rc.s1 = (data[0]<<8 | data[1])/10;
			RC_Ctl.rc.s2 = (data[0]<<8 | data[1])%10;
			arm_flash = data[2];
			last_arm_flash = data[3];
			arm_flashed = data[5];
		}
	#endif
	#ifdef THIRD_CONTROL
	if(RxHead.StdId == 0x100)
		{
			RC_Ctl.rc.s1 = (data[0]<<8 | data[1])/10;
			RC_Ctl.rc.s2 = (data[0]<<8 | data[1])%10;
			arm_flash = data[2];
			last_arm_flash = data[3];
			arm_flashed = data[5];
			arm_rectangle_on = data[6];
			arm_Utype_on = data[7];
		}
	#endif
	#ifdef FOURTH_CONTROL
		if(RxHead.StdId == 0x100)
		{
			RC_Ctl.rc.s1 = (data[0]<<8 | data[1])/10;
			RC_Ctl.rc.s2 = (data[0]<<8 | data[1])%10;
			arm_flash = data[2];
			last_arm_flash = data[3];
			arm_flashed = data[5];
			arm_rectangle_on = data[6];
			arm_Utype_on = data[7];
		}
	#endif
}

