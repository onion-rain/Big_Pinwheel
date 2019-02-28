/** 
* @brief    ȫ����������ʼ��
* @details  This is the detail description. 
* @author   Onion rain
* @date     2018.11.2
* @version  1.0
* @par Copyright (c):  
*       Onion rain
* @par ��־
*/

#ifndef __My_CAR_H
#define	__My_CAR_H

#include "stm32f4xx_hal.h"
#include "Can_Driver.hpp"
#include "Car_Driver.hpp"
#include "cmsis_os.h"

//����࣬pid��
extern softmotor Pinwheel;

void MOTOR_Init_All(void);

#endif
