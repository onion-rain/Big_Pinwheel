/** 
* @brief    ȫ�ֱ�������
* @details  
* @author   Onion rain
* @date     2019.1.15
* @version  1.0
* @par Copyright (c):  Onion rain
* @par ��־
*/

#ifndef __GLOBAL_VARIABLE_H
#define	__GLOBAL_VARIABLE_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

extern TickType_t TimerTicks;
extern uint32_t Pinwheel_angle_set;//�������Ƕ��趨

float limit_float(float out, float min, float max);
int16_t limit_short(int16_t out, int16_t min, int16_t max);

void myprintf(char* fmt, ...);

#endif
