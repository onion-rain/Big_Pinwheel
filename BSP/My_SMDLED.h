/** 
* @brief    SMD型LED板级支持包
* @details  
* @author   Onion rain
* @date     2019.3.4
* @version  1.0
* @par Copyright (c):  Onion rain
* @par 日志
*/

#ifndef __My_SMDLED_H
#define	__My_SMDLED_H

#include "stm32f4xx_hal.h"
#include "tim.h"
#include "define_all.h"

#define GREEN 0x01
#define RED 0x02
#define BLUE 0x04

void SMD_LED_Color_Set(uint8_t color);
void SMD_LED_Init(void);

#endif
