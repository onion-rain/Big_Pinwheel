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
#include <algorithm>

#define MAX_RGB_NUM 72//单列长度

uint8_t Arm_LED_Data[5][5][MAX_RGB_NUM][3] = {0xff};//Arm_LED_Data[风车臂序号][单臂RGB列数][单臂单列RGB数][单RGB LED数]
//uint8_t Arm_index = 0;//风车臂序号(0~3)
uint8_t row_index[5] = {0};//5个臂各自的列指针(0~4)
uint8_t RGB_index[5] = {0};//5个臂各自的RGB指针(0~MAX_RGB_NUM-1)
uint8_t LED_index[5] = {0};//5个臂各自的LED指针(0~2)
uint8_t bit_index[5] = {1};//5个臂各自的数据位指针(0-7),第0个信号在SMD_LED_Color_Set中设置，中断中从第二个信号开始处理

//SMD_LED_Running_Water_Effect_Configuration专属变量
int16_t RGB_Start_index[5][5] = {0};//储存当前周期下各旋臂各列的起始RGB标号

//滑动窗口
static void Sliding_Window(uint8_t arm, uint8_t row, uint8_t mode, uint8_t parameter)
{
	memset(Arm_LED_Data[arm][row], 0x00, sizeof(Arm_LED_Data[arm][row]));//全部清零
	//生成箭头
	if(row==1 || row==3)//1、3列前移一位
		RGB_Start_index[arm][row] = RGB_Start_index[arm][0];//此时row=0这列已经进入下一轮，RGB_Start_index[arm][0]已经加一，故无需在此加一
	else if(row==2)//2列前移两位
		RGB_Start_index[arm][row] = RGB_Start_index[arm][0] + 1;
	//开头、结尾特殊处理
	if(RGB_Start_index[arm][row] < 0)
		memset(Arm_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
	else if(RGB_Start_index[arm][row] > MAX_RGB_NUM-parameter)
		memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, (MAX_RGB_NUM-RGB_Start_index[arm][row])*3);
	else
		memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, parameter*3);
	
	RGB_Start_index[arm][row]++;//累加
	if(RGB_Start_index[arm][row] >= MAX_RGB_NUM)
		RGB_Start_index[arm][row] = -parameter;//溢出归零
	if(row%2 != 0)//1、3列反转
		std::reverse(Arm_LED_Data[arm][row][0], Arm_LED_Data[arm][row][MAX_RGB_NUM]);//此处Arm_LED_Data[arm][row][MAX_RGB_NUM-1]会导致灯条显示bug，暂未知原因
}

//俄罗斯方块
static void Tetris(uint8_t arm, uint8_t row, uint8_t mode, uint8_t parameter)
{
	
}

//传送带
static void Conveyer_Belt(uint8_t arm, uint8_t row, uint8_t mode, uint8_t parameter)
{
	memset(Arm_LED_Data[arm][row], 0x00, sizeof(Arm_LED_Data[arm][row]));//全部清零
	//生成箭头
	if(row==1 || row==3)//1、3列前移一位
		RGB_Start_index[arm][row] = RGB_Start_index[arm][0];//此时row=0这列已经进入下一轮，RGB_Start_index[arm][0]已经加一，故无需在此加一
	else if(row==2)//2列前移两位
		RGB_Start_index[arm][row] = RGB_Start_index[arm][0] + 1;
	
	if(RGB_Start_index[arm][row] > parameter)
		RGB_Start_index[arm][row] -= parameter;
	for(uint8_t i=0; i<=((MAX_RGB_NUM-RGB_Start_index[arm][row])/parameter); i++)//每个亮块的start RGB序号
	{
		if(RGB_Start_index[arm][row]+i*parameter < 0)
			if(i%2 == 0)
				memset(Arm_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
			else memset(Arm_LED_Data[arm][row][0], 0x00, (parameter+RGB_Start_index[arm][row])*3);
		else if(RGB_Start_index[arm][row]+i*parameter > MAX_RGB_NUM-parameter)
			if(i%2 == 0)
				memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]+i*parameter], 0xff, (MAX_RGB_NUM-RGB_Start_index[arm][row]-i*parameter)*3);
			else memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]+i*parameter], 0x00, (MAX_RGB_NUM-RGB_Start_index[arm][row]-i*parameter)*3);
		else
			if(i%2 == 0)
				memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]+i*parameter], 0xff, parameter*3);
			else memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]+i*parameter], 0x00, parameter*3);
	}
	RGB_Start_index[arm][row]++;//累加
	if(RGB_Start_index[arm][row] >= parameter)
		RGB_Start_index[arm][row] = -parameter;//溢出归零
	if(row%2 != 0)//1、3列反转
		std::reverse(Arm_LED_Data[arm][row][0], Arm_LED_Data[arm][row][MAX_RGB_NUM]);//此处Arm_LED_Data[arm][row][MAX_RGB_NUM-1]会导致灯条显示bug，暂未知原因
}

//交叉进度条
static void Progress_Bar_0(uint8_t arm, uint8_t row, uint8_t mode, uint8_t parameter)
{
	if(RGB_Start_index[arm][row] < 0)
		memset(Arm_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
	else if(RGB_Start_index[arm][row] > MAX_RGB_NUM-parameter)
		memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, (MAX_RGB_NUM-RGB_Start_index[arm][row])*3);
	else
		memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, parameter*3);
	RGB_Start_index[arm][row]++;//累加
	if(RGB_Start_index[arm][row] >= MAX_RGB_NUM)
		RGB_Start_index[arm][row] = -parameter;//溢出归零
}

//同向进度条
static void Progress_Bar_1(uint8_t arm, uint8_t row, uint8_t mode, uint8_t parameter)
{
	
}

/** @brief  灯条流水灯效设置
	* @param	[in]  arm	灯臂标号
	* @param	[in]  mode 模式
  *					This parameter can be one of the following values:
	*         @arg ALL_ON: 全亮(parameter无意义)
	*         @arg SLIDING_WINDOW: 滑动窗口(parameter为窗口大小)
	*         @arg TETRIS: 俄罗斯方块(parameter方块大小)
	*         @arg CONVEYER_BELT: 传送带(parameter为暗块/亮块宽度)
	*         @arg PROGRESS_BAR_0: 交叉进度条(parameter为进度条每次更新增长的长度)
	*         @arg PROGRESS_BAR_1: 同向进度条(parameter为进度条每次更新增长的长度)
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
//	uint8_t row = 0;//当前臂列序号
	//将每个RGB的亮灭配置信息放入Arm_LED_Data
	for(uint8_t row=0; row<5; row++)
	{
		switch(mode)
		{
			case ALL_ON:
				memset(Arm_LED_Data[arm][row], 0xff, sizeof(Arm_LED_Data[arm][row]));//全部置一
				break;
			case SLIDING_WINDOW://滑动窗口
				Sliding_Window(arm, row, mode, parameter);
				break;
			case TETRIS://俄罗斯方块
				Tetris(arm, row, mode, parameter);
				break;
			case CONVEYER_BELT://传送带
				Conveyer_Belt(arm, row, mode, parameter);
				break;
			case PROGRESS_BAR_0://交叉进度条
				Progress_Bar_0(arm, row, mode, parameter);
				break;
			case PROGRESS_BAR_1://同向进度条
				Progress_Bar_1(arm, row, mode, parameter);
				break;
		}
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
	if(Arm_LED_Data[1][row_index[1]][RGB_index[1]][LED_index[1]] == 0xff)
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





