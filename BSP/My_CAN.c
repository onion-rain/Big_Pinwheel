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

CAN_RxHeaderTypeDef RxHead;

static uint8_t data[8];//�������ݻ�����

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&RxHead,data);
	manager::CANUpdate(hcan,&RxHead,data);
}

