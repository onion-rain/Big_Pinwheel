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

#define RED 0x00ff0000//纯色高八位为0
#define GREEN 0x0000ff00
#define BLUE 0x000000ff
#define ORANGE 0xff8f00
#define CYAN 0x00ffff
#define ORANGE_x 0x7F2000
#define CYAN_x 0x006043

#define RAND 0x10000000
#define GRADATION 0x20000000
#define RUNNING_WATER 0x30000000

//大符臂流水灯阵灯效
#define ALL_ON 0x00
#define SLIDING_WINDOW 0x01
#define CONVEYER_BELT 0x02
#define PROGRESS_BAR_0 0x03
#define PROGRESS_BAR_1 0x04
#define PROGRESS_BAR_2 0x05
#define TETRIS 0x06

//大符臂外围灯效
#define UNSETLECTED 0x00
#define WAIT_HIT 0x01
#define HITTED 0x02
#define SUCCESS 0x03

uint8_t ARM_Inside_ligthting_effect(uint8_t arm, uint8_t mode, uint8_t parameter, uint32_t color);
void ARM_Outside_ligthting_effect(uint8_t arm, uint8_t mode, uint32_t color);
void SMD_INSIDE_LED_IT(void);
void SMD_OUTSIDE_LED_IT(void);
void SMD_LED_PWM_Init(void);

#endif
