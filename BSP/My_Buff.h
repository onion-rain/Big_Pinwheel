/** 
* @brief    大符
* @details  
* @author   Onion rain
* @date     2019.3.21
* @version  1.0
* @par Copyright (c):  Onion rain
* @par 日志
*/

#ifndef __My_BUFF_H
#define	__My_BUFF_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

extern uint8_t arm_flash, last_arm_flash, arm_flashed, arm_rectangle_on, arm_Utype_on;
#ifdef MASTER_CONTROL//主控
	extern uint8_t hit[17];//记录五个装甲板的打击次数
	extern uint16_t Unprogrammable_Light_Bar;
#endif
extern TickType_t LastShootTick[];

extern void clear_with_purity_color(uint8_t color);
extern void buff_conveyer_belt(void);
extern void buff_all_on(void);
extern void buff_reset(void);
extern void buff_flash(void);
extern void buff_new_armnum_produce(void);

#endif
