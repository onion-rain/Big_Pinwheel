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

CAN_RxHeaderTypeDef RxHead;

static uint8_t data[8];//接收数据缓冲区

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&RxHead,data);
	manager::CANUpdate(hcan,&RxHead,data);
}

