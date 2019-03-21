/** 
* @brief    ´ó·û
* @details  
* @author   Onion rain
* @date     2019.3.21
* @version  1.0
* @par Copyright (c):  Onion rain
* @par ÈÕÖ¾
*/

#ifndef __My_BUFF_H
#define	__My_BUFF_H

#include "stm32f4xx_hal.h"

extern uint8_t arm_flash;

extern void clear_with_purity_color(uint8_t color);
extern void buff_conveyer_belt(void);
extern void buff_all_on(void);
extern void buff_flag_reset(void);
extern void buff_flash(void);
extern void buff_new_armnum_produce(void);

#endif
