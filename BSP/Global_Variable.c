/** 
* @brief    全局变量汇总
* @details  以及常用通用函数
* @author   Onion rain
* @date     2019.1.15
* @version  1.0
* @par Copyright (c):  Onion rain
* @par 日志
*/

#include "Global_Variable.h"
#include <stdarg.h>
#include "usart.h"
#include "define_all.h"

TickType_t TimerTicks;//系统心跳,在主线程中更新
uint32_t Pinwheel_angle_set;//大符电机角度设定
uint32_t Pinwheel_speed_set;//大符电机速度设定

int16_t can_buffer[4] = {0};//副控发送缓冲池
//can_buffer[0]发送遥控器模式
//can_buffer[1]亮起臂选择(前8位作为臂选，后8位控制每个臂处于未被击打和已被击打的模式)

float limit_float(float out, float min, float max)
{
	if(out > max)
			return max;
	else if(out < min)
			return min;
	else return out;
}

int16_t limit_short(int16_t out, int16_t min, int16_t max)
{
	if(out > max)
			return max;
	else if(out < min)
			return min;
	else return out;
}

