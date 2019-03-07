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
#include <stdlib.h>

int tmp = 0;

#define MAX_RGB_NUM 72//单列长度

uint8_t Arm_LED_Data[5][5][MAX_RGB_NUM][3] = {0xff};//Arm_LED_Data[风车臂序号][单臂RGB列数][单臂单列RGB数][单RGB LED数]
uint8_t row_index[5] = {0};//5个臂各自的列指针(0~4)
uint8_t RGB_index[5] = {0};//5个臂各自的RGB指针(0~MAX_RGB_NUM-1)
uint8_t LED_index[5] = {0};//5个臂各自的LED指针(0~2)
uint8_t bit_index[5] = {1};//5个臂各自的数据位指针(0-7),第0个信号在SMD_LED_Color_Set中设置，中断中从第二个信号开始处理

//SMD_LED_Running_Water_Effect_Configuration专属变量
int8_t RGB_Start_index[5][5] = {0};//储存当前周期下各旋臂各列的起始亮RGB标号
uint8_t RGB_Tail_num[5][5] = {0};//储存当前周期下各旋臂各列的尾部聚集的发光RGB数量(滴水进度条、俄罗斯方块专属)

//滑动窗口
static uint8_t Sliding_Window(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<5; row++)
	{
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
	uint8_t return_num = MAX_RGB_NUM+1-RGB_Start_index[arm][2]-parameter;//返回第三列尖端位置
	if(return_num > MAX_RGB_NUM)
		return 0;
	else return return_num;
}
//传送带
static uint8_t Conveyer_Belt(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<5; row++)
	{
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
	return 0;
}
//交叉进度条
static uint8_t Progress_Bar_0(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<5; row++)
	{
		memset(Arm_LED_Data[arm][row][0], 0xff, RGB_Start_index[arm][row]*3);
		if(RGB_Start_index[arm][row] < MAX_RGB_NUM)
			RGB_Start_index[arm][row]++;//累加
	}
	return MAX_RGB_NUM-RGB_Start_index[arm][2];
}
//同向进度条
static uint8_t Progress_Bar_1(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<5; row++)
	{
		memset(Arm_LED_Data[arm][row][0], 0xff, RGB_Start_index[arm][row]*3);
		if(RGB_Start_index[arm][row] < MAX_RGB_NUM)
			RGB_Start_index[arm][row]++;//累加
		if(row%2 != 0)//1、3列反转
			std::reverse(Arm_LED_Data[arm][row][0], Arm_LED_Data[arm][row][MAX_RGB_NUM]);//此处Arm_LED_Data[arm][row][MAX_RGB_NUM-1]会导致灯条显示bug，暂未知原因
	}
	return MAX_RGB_NUM-RGB_Start_index[arm][2];
}
//滴水进度条
static uint8_t Progress_Bar_2(uint8_t arm, uint8_t parameter)//该函数里可能有指针指飞，进度条满后一段时间不操作会导致死在tim8的中断里，或者进errorhandle
{
	for(uint8_t row=0; row<5; row++)
	{
		//开头、结尾特殊处理
		if(RGB_Start_index[arm][row] < 0)
			memset(Arm_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
		else if(RGB_Start_index[arm][row] > MAX_RGB_NUM-parameter)
			memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, (MAX_RGB_NUM-RGB_Start_index[arm][row])*3);
		else
			memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, parameter*3);
		RGB_Start_index[arm][row]++;//累加
		if(RGB_Start_index[arm][row] > MAX_RGB_NUM-RGB_Tail_num[arm][row]-parameter && RGB_Tail_num[arm][row] < MAX_RGB_NUM)
		{
			RGB_Start_index[arm][row] = -parameter;//溢出归零
			RGB_Tail_num[arm][row] += parameter;
		}
		if(RGB_Tail_num[arm][row] != 0)
			memset(Arm_LED_Data[arm][row][MAX_RGB_NUM-RGB_Tail_num[arm][row]], 0xff, RGB_Tail_num[arm][row]*3);
		if(row%2 != 0)//1、3列反转
			std::reverse(Arm_LED_Data[arm][row][0], Arm_LED_Data[arm][row][MAX_RGB_NUM]);//此处Arm_LED_Data[arm][row][MAX_RGB_NUM-1]会导致灯条显示bug，暂未知原因
		if(RGB_Tail_num[arm][row] >= MAX_RGB_NUM)
			RGB_Tail_num[arm][row] = MAX_RGB_NUM;
	}
	return MAX_RGB_NUM-RGB_Tail_num[arm][3];
}
//俄罗斯方块
static uint8_t Tetris(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<5; row++)
	{
		
	}
	return 0;
}

/** @brief  灯条流水灯效设置
	* @param	[in]  arm	灯臂标号
	* @param	[in]  mode 模式
  *					This parameter can be one of the following values:
	*         @arg ALL_ON: 全亮(parameter无意义)(返回值无意义)
	*         @arg SLIDING_WINDOW: 滑动窗口(parameter为窗口大小)(返回滑块尖端距末端距离)
	*         @arg CONVEYER_BELT: 传送带(parameter为暗块/亮块宽度)(返回值无意义)
	*         @arg PROGRESS_BAR_0: 交叉进度条(parameter无意义)(返回进度信息)
	*         @arg PROGRESS_BAR_1: 同向进度条(parameter无意义)(返回进度信息)
	*         @arg PROGRESS_BAR_3: 滴水进度条(parameter为水滴长度)(返回进度信息)
	*         @arg TETRIS: 俄罗斯方块(parameter方块大小)(返回是否游戏失败,0继续/1失败)
	* @param	[in]  parameter 参数
	* @param	[in]  color 显示的颜色
  *					This parameter can be one of the following values:
	*         @arg GREEN: 纯色绿色
	*         @arg RED: 	纯色红色
	*         @arg BLUE:  纯色蓝色(纯色间可用|自由搭配)
	*         @arg RAND:	随机色
	*         @arg RUNNING_WATER:	流水灯效
	* @details	周期调用该函数以达到流水效果
	* @retval 不同mode含义不同
	*/
uint8_t SMD_LED_Running_Water_Effect_Configuration(uint8_t arm, uint8_t mode, uint8_t parameter, uint8_t color)
{
	uint8_t return_data;
	//将每个RGB的亮灭配置信息放入Arm_LED_Data
	memset(Arm_LED_Data[arm], 0x00, sizeof(Arm_LED_Data[arm]));//全部清零
	switch(mode)
	{
		case ALL_ON:
			memset(Arm_LED_Data[arm], 0xff, sizeof(Arm_LED_Data[arm]));//全部置一
			return_data = 0;
			break;
		case SLIDING_WINDOW://滑动窗口
			return_data = Sliding_Window(arm, parameter);
			break;
		case CONVEYER_BELT://传送带
			return_data = Conveyer_Belt(arm, parameter);
			break;
		case PROGRESS_BAR_0://交叉进度条
			return_data = Progress_Bar_0(arm, parameter);
			break;
		case PROGRESS_BAR_1://同向进度条
			return_data = Progress_Bar_1(arm, parameter);
			break;
		case PROGRESS_BAR_2://滴水进度条
			return_data = Progress_Bar_2(arm, parameter);
			break;
		case TETRIS://俄罗斯方块
			return_data = Tetris(arm, parameter);
			break;
	}
	//将颜色信息添加入Arm_LED_Data
	uint8_t color_set = 0;
	for(uint8_t row=0; row<5; row++)
		for(uint8_t led=0; led<MAX_RGB_NUM; led++)
		{
			switch(color & 0xf8)
			{
				case 0:color_set = color;break;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
				case RAND:color_set = rand()%6+1;break;
				case GRADATION:break;
				case RUNNING_WATER:break;
			}
			if(!(color_set & GREEN))
				Arm_LED_Data[arm][row][led][0] = 0x00;
			if(!(color_set & RED))
				Arm_LED_Data[arm][row][led][1] = 0x00;
			if(!(color_set & BLUE))
				Arm_LED_Data[arm][row][led][2] = 0x00;
		}
	//设置第一列第一个RGB中的绿色LED pwm脉冲占空比，
	if(Arm_LED_Data[arm][0][LED_index[1]/3][LED_index[1]%3] == 0xff)
		ARM1_PULSE = LOGIC_ONE_PULSE;
	else ARM1_PULSE = LOGIC_ZERO_PULSE;
	//开启pwm中断
	__HAL_TIM_ENABLE_IT(&htim2,TIM_IT_UPDATE);
	return return_data;
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





