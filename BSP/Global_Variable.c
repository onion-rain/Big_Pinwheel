/** 
* @brief    ȫ�ֱ�������
* @details  �Լ�����ͨ�ú���
* @author   Onion rain
* @date     2019.1.15
* @version  1.0
* @par Copyright (c):  Onion rain
* @par ��־
*/

#include "Global_Variable.h"
#include <stdarg.h>
#include "usart.h"
#include "define_all.h"

TickType_t TimerTicks;//ϵͳ����,�����߳��и���
uint32_t Pinwheel_angle_set;//�������Ƕ��趨
uint32_t Pinwheel_speed_set;//�������ٶ��趨

int16_t can_buffer[4] = {0};//���ط��ͻ����
//can_buffer[0]����ң����ģʽ
//can_buffer[1]�����ѡ��(ǰ8λ��Ϊ��ѡ����8λ����ÿ���۴���δ��������ѱ������ģʽ)

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

