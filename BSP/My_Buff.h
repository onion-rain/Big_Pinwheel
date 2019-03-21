/** 
* @brief    SMD型LED板级支持包
* @details  
* @author   Onion rain
* @date     2019.3.4
* @version  1.0
* @par Copyright (c):  Onion rain
* @par 日志
*/

#ifndef __My_BUFF_H
#define	__My_BUFF_H

#include "stm32f4xx_hal.h"

extern uint8_t arm_flash;
extern void buff_conveyer_belt(void);
extern void buff_all_on(void);
extern void buff_reset(void);
extern void buff_flash(void);
extern void buff_new_armnum_produce(void);

#endif
