/** 
* @brief    DBUS板级支持包
* @details  DMA接收
* @author   Onion rain
* @date     date 2018.10.15
* @version  1.0
* @par Copyright (c):  Onion rain
* @par 日志
*/  
#ifndef __DBUS_UART_H
#define __DBUS_UART_H

#include "stm32f4xx.h"

//鼠标键值 自定义的
#define MOUSE_L (0x0000)
#define MOUSE_R (0xffff)

//键盘键值
#define KEY_W			(0x0001)
#define KEY_S			(0x0002)
#define KEY_A			(0x0004)
#define KEY_D			(0x0008)
#define KEY_Q			(0x0040)
#define KEY_E			(0x0080)
#define KEY_SHIFT		(0x0010)
#define KEY_Ctrl		(0x0020)

#define KEY_B   (0x8000) 
#define KEY_V		(0x4000) 
#define KEY_C		(0x2000) 
#define KEY_X		(0x1000) 
#define KEY_Z		(0x0800) 
#define KEY_G		(0x0400) 
#define KEY_F		(0x0200)
#define KEY_R		(0x0100)

typedef struct
{
	struct
	{
		int16_t ch0;
		int16_t ch1;
		int16_t ch2;
		int16_t ch3;
		uint8_t s1;
		uint8_t s2;
	}rc;
	struct
	{
		int16_t x;
		int16_t y;
		int16_t z;
		uint8_t press_l;
		uint8_t press_r;
	}mouse;
	struct
	{
		uint16_t v;
	}key;
}RC_Ctl_t;

extern RC_Ctl_t RC_Ctl;//解析后

void Dbus_Uart_Init(void);
void Dbus_Uart_IT(void);

#endif


