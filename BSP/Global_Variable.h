/** 
* @brief    全局变量汇总
* @details  
* @author   Onion rain
* @date     2019.1.15
* @version  1.0
* @par Copyright (c):  Onion rain
* @par 日志
*/

#ifndef __GLOBAL_VARIABLE_H
#define	__GLOBAL_VARIABLE_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

extern TickType_t TimerTicks, DbusTick;//系统心跳,在主线程中更新
extern uint8_t DbusOutofContact;//各系统失恋标志位,默认全部失恋
extern uint32_t Pinwheel_angle_set;//大符电机角度设定
extern uint32_t Pinwheel_speed_set;//大符电机速度设定

extern int16_t can_buffer[4];

float limit_float(float out, float min, float max);
int16_t limit_short(int16_t out, int16_t min, int16_t max);

void myprintf(char* fmt, ...);

#endif
