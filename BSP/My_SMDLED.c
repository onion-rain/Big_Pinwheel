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
#include "string.h"
#include "tim.h"

#define MAX_LED_NUM 288

uint8_t Arm_RGB_Data[5][MAX_LED_NUM][3] = {0xff};
int16_t LED_Configuration_index[5] = {0};//SMD_LED_Running_Water_Effect_Configuration中的临时变量中的临时变量，储存当前周期下的起始灯标号
//uint8_t Arm_index = 0;//五个臂代号(0~3)
uint16_t LED_index[5] = {0};//5个臂各自的灯号数(0~144*3)
uint8_t bit_index = 1;//每个颜色8位(0~7)//第0个信号在SMD_LED_Color_Set中设置，中断中从第二个信号开始处理

/** @brief  灯条流水灯效设置
	* @param	[in]  arm	灯臂标号
	* @param	[in]  mode 模式
  *					This parameter can be one of the following values:
	*         @arg ALL_ON: 全亮(parameter无意义)
	*         @arg SLIDING_WINDOW: 滑动窗口(parameter为窗口大小)
	*         @arg TETRIS: 俄罗斯方块(parameter方块大小)
	*         @arg CONVEYER_BELT: 传送带(parameter为暗块/亮块宽度)
	* @param	[in]  parameter 参数
	* @param	[in]  color 显示的颜色
  *					This parameter can be one of the following values:
	*         @arg GREEN: 纯色绿色
	*         @arg RED: 	纯色红色
	*         @arg BLUE:  纯色蓝色(纯色间可用|自由搭配)
	*         @arg RAND:	随机色
	*         @arg RUNNING_WATER:	流水灯效
	* @details	周期调用该函数以达到流水效果
	* @retval None
	*/
void SMD_LED_Running_Water_Effect_Configuration(uint8_t arm, uint8_t mode, uint8_t parameter, uint8_t color)
{
	memset(Arm_RGB_Data[arm], 0x00, sizeof(Arm_RGB_Data[arm]));
	switch(mode)
	{
		case ALL_ON:
			memset(Arm_RGB_Data[arm][0], 0xff, MAX_LED_NUM*3);
			break;
		case SLIDING_WINDOW:
			if(LED_Configuration_index[arm] < 0)
				memset(Arm_RGB_Data[arm][0], 0xff, (parameter+LED_Configuration_index[arm])*3);
//				memset(Arm_RGB_Data[arm][MAX_LED_NUM+LED_Configuration_index[arm]], 0xff, -LED_Configuration_index[arm]*3);
			else if(LED_Configuration_index[arm] > MAX_LED_NUM-parameter)
				memset(Arm_RGB_Data[arm][LED_Configuration_index[arm]], 0xff, (MAX_LED_NUM-LED_Configuration_index[arm])*3);
			else
				memset(Arm_RGB_Data[arm][LED_Configuration_index[arm]], 0xff, parameter*3);
			LED_Configuration_index[arm]++;//累加
			if(LED_Configuration_index[arm] >= MAX_LED_NUM)
				LED_Configuration_index[arm] = -parameter;//溢出归零
			break;
		case TETRIS:
			break;
		case CONVEYER_BELT:
			for(uint8_t i=0; i<=((MAX_LED_NUM-LED_Configuration_index[arm])/parameter); i++)
			{
				if(LED_Configuration_index[arm]+i*parameter < 0)
					if(i%2 == 0)
						memset(Arm_RGB_Data[arm][0], 0xff, (parameter+LED_Configuration_index[arm])*3);
					else memset(Arm_RGB_Data[arm][0], 0x00, (parameter+LED_Configuration_index[arm])*3);
				else if(LED_Configuration_index[arm]+i*parameter > MAX_LED_NUM-parameter)
					if(i%2 == 0)
						memset(Arm_RGB_Data[arm][LED_Configuration_index[arm]+i*parameter], 0xff, (MAX_LED_NUM-LED_Configuration_index[arm]-i*parameter)*3);
					else memset(Arm_RGB_Data[arm][LED_Configuration_index[arm]+i*parameter], 0x00, (MAX_LED_NUM-LED_Configuration_index[arm]-i*parameter)*3);
				else
					if(i%2 == 0)
						memset(Arm_RGB_Data[arm][LED_Configuration_index[arm]+i*parameter], 0xff, parameter*3);
					else memset(Arm_RGB_Data[arm][LED_Configuration_index[arm]+i*parameter], 0x00, parameter*3);
			}
			LED_Configuration_index[arm]++;//累加
			if(LED_Configuration_index[arm] >= parameter)
				LED_Configuration_index[arm] = -parameter;//溢出归零
			break;
	}
	
	for(int arm=0; arm<5; arm++)
		for(int led=0; led<MAX_LED_NUM; led++)
		{
			if(!(color & GREEN))
				Arm_RGB_Data[arm][led][0] = 0x00;
			if(!(color & RED))
				Arm_RGB_Data[arm][led][1] = 0x00;
			if(!(color & BLUE))
				Arm_RGB_Data[arm][led][2] = 0x00;
		}
	//设置第一个pwm脉冲占空比
	if(Arm_RGB_Data[1][LED_index[1]/3][LED_index[1]%3] == 0xff)
		ARM1_PULSE = LOGIC_ONE_PULSE;
	else ARM1_PULSE = LOGIC_ZERO_PULSE;
	//开启pwm输出
	__HAL_TIM_ENABLE_IT(&htim2,TIM_IT_UPDATE);
//	HAL_TIM_PWM_Start(ARM0_TIM,ARM0_CHANNEL);
//	HAL_TIM_PWM_Start(ARM1_TIM,ARM1_CHANNEL);
//	HAL_TIM_PWM_Start(ARM2_TIM,ARM2_CHANNEL);
//	HAL_TIM_PWM_Start(ARM3_TIM,ARM3_CHANNEL);
//	HAL_TIM_PWM_Start(ARM4_TIM,ARM4_CHANNEL);
}

void SMD_LED_TIM2_IT(void)
{
	if(Arm_RGB_Data[1][LED_index[1]/3][LED_index[1]%3] == 0xff)
		ARM1_PULSE = LOGIC_ONE_PULSE;
	else ARM1_PULSE = LOGIC_ZERO_PULSE;
	bit_index++;
	if(bit_index == 8)//一个灯的8位数据遍历完
	{
		bit_index = 0;
		LED_index[1]++;
	}
	if(LED_index[1] == MAX_LED_NUM*3+1)//加一补上最后8位蓝灯
	{
//		Arm_index = 0;
		LED_index[1] = 0;
		bit_index = 1;
//		HAL_TIM_PWM_Stop(ARM1_TIM,ARM1_CHANNEL);
		__HAL_TIM_DISABLE_IT(&htim2,TIM_IT_UPDATE);
		ARM1_PULSE = 0;
	}
}





