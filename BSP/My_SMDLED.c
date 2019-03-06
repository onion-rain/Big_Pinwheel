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

#define MAX_RGB_NUM 72//单列长度

uint8_t Arm_LED_Data[5][5][MAX_RGB_NUM][3] = {0xff};//Arm_LED_Data[风车臂序号][单臂RGB列数][单臂单列RGB数][单RGB LED数]
//uint8_t Arm_index = 0;//风车臂序号(0~3)
uint8_t row_index[5] = {0};//5个臂各自的列指针(0~4)
uint8_t RGB_index[5] = {0};//5个臂各自的RGB指针(0~MAX_RGB_NUM-1)
uint8_t LED_index[5] = {0};//5个臂各自的LED指针(0~2)
uint8_t bit_index[5] = {1};//5个臂各自的数据位指针(0-7),第0个信号在SMD_LED_Color_Set中设置，中断中从第二个信号开始处理

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
	int16_t RGB_ON_OFF_Configuration_index[5] = {0};//储存当前周期下各旋臂的起始RGB标号
	uint8_t row = 0;//当前臂列序号
	memset(Arm_LED_Data[arm], 0x00, sizeof(Arm_LED_Data[arm]));//全部清零
	//将每个RGB的亮灭配置信息放入Arm_LED_Data
	for(; row<5; row++)
		switch(mode)
		{
			case ALL_ON:
				memset(Arm_LED_Data[arm][row], 0xff, sizeof(Arm_LED_Data[arm][row]));//全部置一
				break;
			case SLIDING_WINDOW:
//				if(RGB_ON_OFF_Configuration_index[arm] < 0)
//					memset(Arm_LED_Data[arm][row][0], 0xff, (parameter+RGB_ON_OFF_Configuration_index[arm])*3);
//	//				memset(Arm_LED_Data[arm][MAX_RGB_NUM+RGB_ON_OFF_Configuration_index[arm]], 0xff, -RGB_ON_OFF_Configuration_index[arm]*3);
//				else if(RGB_ON_OFF_Configuration_index[arm] > MAX_RGB_NUM-parameter)
//					memset(Arm_LED_Data[arm][row][RGB_ON_OFF_Configuration_index[arm]], 0xff, (MAX_RGB_NUM-RGB_ON_OFF_Configuration_index[arm])*3);
//				else
//					memset(Arm_LED_Data[arm][row][RGB_ON_OFF_Configuration_index[arm]], 0xff, parameter*3);
//				RGB_ON_OFF_Configuration_index[arm]++;//累加
//				if(RGB_ON_OFF_Configuration_index[arm] >= MAX_RGB_NUM)
//					RGB_ON_OFF_Configuration_index[arm] = -parameter;//溢出归零
				break;
			case TETRIS:
				break;
			case CONVEYER_BELT:
	//			for(uint8_t i=0; i<=((MAX_RGB_NUM-RGB_ON_OFF_Configuration_index[arm])/parameter); i++)
	//			{
	//				if(RGB_ON_OFF_Configuration_index[arm]+i*parameter < 0)
	//					if(i%2 == 0)
	//						memset(Arm_LED_Data[arm][0], 0xff, (parameter+RGB_ON_OFF_Configuration_index[arm])*3);
	//					else memset(Arm_LED_Data[arm][0], 0x00, (parameter+RGB_ON_OFF_Configuration_index[arm])*3);
	//				else if(RGB_ON_OFF_Configuration_index[arm]+i*parameter > MAX_RGB_NUM-parameter)
	//					if(i%2 == 0)
	//						memset(Arm_LED_Data[arm][RGB_ON_OFF_Configuration_index[arm]+i*parameter], 0xff, (MAX_RGB_NUM-RGB_ON_OFF_Configuration_index[arm]-i*parameter)*3);
	//					else memset(Arm_LED_Data[arm][RGB_ON_OFF_Configuration_index[arm]+i*parameter], 0x00, (MAX_RGB_NUM-RGB_ON_OFF_Configuration_index[arm]-i*parameter)*3);
	//				else
	//					if(i%2 == 0)
	//						memset(Arm_LED_Data[arm][RGB_ON_OFF_Configuration_index[arm]+i*parameter], 0xff, parameter*3);
	//					else memset(Arm_LED_Data[arm][RGB_ON_OFF_Configuration_index[arm]+i*parameter], 0x00, parameter*3);
	//			}
	//			RGB_ON_OFF_Configuration_index[arm]++;//累加
	//			if(RGB_ON_OFF_Configuration_index[arm] >= parameter)
	//				RGB_ON_OFF_Configuration_index[arm] = -parameter;//溢出归零
				break;
		}
	//将颜色信息添加入Arm_LED_Data
	for(int row=0; row<5; row++)
		for(int led=0; led<MAX_RGB_NUM; led++)
		{
			if(!(color & GREEN))
				Arm_LED_Data[arm][row][led][0] = 0x00;
			if(!(color & RED))
				Arm_LED_Data[arm][row][led][1] = 0x00;
			if(!(color & BLUE))
				Arm_LED_Data[arm][row][led][2] = 0x00;
		}
	//设置第一列第一个RGB中的绿色LED pwm脉冲占空比，
	if(Arm_LED_Data[arm][0][LED_index[1]/3][LED_index[1]%3] == 0xff)
		ARM1_PULSE = LOGIC_ONE_PULSE;
	else ARM1_PULSE = LOGIC_ZERO_PULSE;
	//开启pwm中断
	__HAL_TIM_ENABLE_IT(&htim2,TIM_IT_UPDATE);
}

void SMD_LED_TIM2_IT(void)
{
	if(Arm_LED_Data[1][row_index[1]][LED_index[1]/3][LED_index[1]%3] == 0xff)
		ARM1_PULSE = LOGIC_ONE_PULSE;
	else ARM1_PULSE = LOGIC_ZERO_PULSE;
	bit_index[1]++;
	if(bit_index[1] == 8)//一个LED的8位数据遍历完
	{
		bit_index[1] = 0;//数据位指针归零
		LED_index[1]++;//下一个LED
		if(LED_index[1] == 3)
		{
			LED_index[1] = 0;//LED指针清零
			RGB_index[1]++;//下一个RGB
			if(RGB_index[1] == MAX_RGB_NUM)
			{
				RGB_index[1] = 0;//RGB指针清零
				row_index[1]++;//下一列
				if(row_index[1] == 5)
				{
					__HAL_TIM_DISABLE_IT(&htim2,TIM_IT_UPDATE);//关中断
					ARM1_PULSE = 0;//IO拉低
					row_index[1] = 0;//列指针清零
					bit_index[1] = 1;//当遍历完一个臂上所有RGB的所有LED的所有位，下次进中断设置的应是第二个LED的占空比，故此处为1
				}
			}
		}
	}
}





