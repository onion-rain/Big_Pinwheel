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
#define RAND 0x08
#define GRADATION 0x10
#define RUNNING_WATER 0x18

#define ALL_ON 0x00
#define SLIDING_WINDOW 0x01
#define CONVEYER_BELT 0x02
#define PROGRESS_BAR_0 0x03
#define PROGRESS_BAR_1 0x04
#define PROGRESS_BAR_2 0x05
#define TETRIS 0x06

uint8_t SMD_LED_Running_Water_Effect_Configuration(uint8_t arm, uint8_t mode, uint8_t parameter, uint8_t color);
void SMD_LED_IT(void);

#endif
