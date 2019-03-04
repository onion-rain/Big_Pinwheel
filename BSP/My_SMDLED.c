/** 
* @brief    SMD型LED板级支持包
* @details  
* @author   Onion rain
* @date     2019.3.4
* @version  1.0
* @par Copyright (c):  Onion rain
* @par 日志
*/

#include "My_SMDLED.h"

uint8_t Arm_Data[5][144][3] = {0xff};
uint16_t LED_index[5] = {0};

void SMD_LED_Color_Set(uint8_t color)
{
	 for(int arm=0; arm<5; arm++)
			for(int n=0; n<144; n++)
			{
				if(color & GREEN)
					Arm_Data[arm][n][0] = 0xff;
				else Arm_Data[arm][n][0] = 0x00;
				if(color & RED)
					Arm_Data[arm][n][1] = 0xff;
				else Arm_Data[arm][n][1] = 0x00;
				if(color & BLUE)
					Arm_Data[arm][n][2] = 0xff;
				else Arm_Data[arm][n][2] = 0x00;
			}
}

void SMD_LED_Init(void)
{
	 if(Arm_Data[0][LED_index[0]/3][LED_index[0]%3] == 0xff)
		ARM1_PULSE = LOGIC_ONE_PULSE;
	 else ARM1_PULSE = LOGIC_ZERO_PULSE;
	 LED_index[0]++;
	 if(LED_index[0] == 144)
	 {
		 LED_index[0] = 0;
			//关闭pwm
	 }
}
