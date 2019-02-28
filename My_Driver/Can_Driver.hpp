/** 
* @file         Can_Driver.hpp 
* @author      WMD
* @version  1.0
* @par Copyright (c):  
*       WMD 
* @par 日志见cpp
*/  
#ifndef _CAN_DRIVER_H
#define _CAN_DRIVER_H

#include "CarDrv_config.hpp"

void CAN_Init_All(void);
HAL_StatusTypeDef CAN_SEND_PROTECT(CAN_HandleTypeDef *_hcan,int id);
HAL_StatusTypeDef can_send_msg(CAN_HandleTypeDef* _hcan, int id, int16_t* s16buff);


#endif
