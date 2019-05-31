/** 
* @brief    SMD型LED板级支持包
* @details  SMD_INSIDE_LED_IT()需要放到时钟中断回调函数中
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
#include "define_all.h"

#define ARM_PER_BOARD 3//每个主控最大控制臂个数
#define ROW_PER_ARM 5//单臂列数
#define RGB_PER_ROW 64//单列RGB数
#define ARM_UTYPE_LENGTH 100//大符臂外围灯条长度
#define ARM_RECTANGLE_LENGTH 123//矩形框灯条长度

uint8_t Arm_Outside_LED_Data[ARM_PER_BOARD][1][ARM_UTYPE_LENGTH+ARM_RECTANGLE_LENGTH][3] = {0xff};//Arm_Outside_LED_Data[风车臂序号][单臂RGB数/2][单RGB LED数]
uint8_t Outside_row_index[ARM_PER_BOARD] = {0};//5个臂各自的列指针(0)
uint8_t Outside_RGB_index[ARM_PER_BOARD] = {0};//5个臂各自的RGB指针(0~RGB_PER_ROW-1)
uint8_t Outside_LED_index[ARM_PER_BOARD] = {0};//5个臂各自的LED指针(0~2)
uint8_t Outside_bit_index[ARM_PER_BOARD] = {1};//5个臂各自的数据位指针(0-7),第0个信号在SMD_LED_Color_Set中设置，中断中从第二个信号开始处理
uint8_t Arm_Inside_LED_Data[ARM_PER_BOARD][ROW_PER_ARM][RGB_PER_ROW][3] = {0xff};//Arm_Inside_LED_Data[风车臂序号][单臂RGB列数][单臂单列RGB数][单RGB LED数]
uint8_t Inside_row_index[ARM_PER_BOARD] = {0};//5个臂各自的列指针(0~4)
uint8_t Inside_RGB_index[ARM_PER_BOARD] = {0};//5个臂各自的RGB指针(0~RGB_PER_ROW-1)
uint8_t Inside_LED_index[ARM_PER_BOARD] = {0};//5个臂各自的LED指针(0~2)
uint8_t Inside_bit_index[ARM_PER_BOARD] = {1};//5个臂各自的数据位指针(0-7),第0个信号在SMD_LED_Color_Set中设置，中断中从第二个信号开始处理

//ARM_Inside_ligthting_effect专属变量
int8_t RGB_Start_index[5][ROW_PER_ARM] = {0};//储存当前周期下各旋臂各列的起始亮RGB标号
uint8_t RGB_Tail_num[5][ROW_PER_ARM] = {0};//储存当前周期下各旋臂各列的尾部聚集的发光RGB数量(滴水进度条、俄罗斯方块专属)
uint8_t RGB_success_schedule[5] = {0};//打符成功边缘灯条进度条进度信息

//滑动窗口
static uint8_t Sliding_Window(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
	{
		//生成箭头
		if(row==1 || row==3)//1、3列前移一位
			RGB_Start_index[arm][row] = RGB_Start_index[arm][0];//此时row=0这列已经进入下一轮，RGB_Start_index[arm][0]已经加一，故无需在此加一
		else if(row==2)//2列前移两位
			RGB_Start_index[arm][row] = RGB_Start_index[arm][0] + 1;
		//开头、结尾特殊处理
		if(RGB_Start_index[arm][row] < 0)
			memset(Arm_Inside_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
		else if(RGB_Start_index[arm][row] > RGB_PER_ROW-parameter)
			memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, (RGB_PER_ROW-RGB_Start_index[arm][row])*3);
		else
			memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, parameter*3);
		RGB_Start_index[arm][row]++;//累加
		if(RGB_Start_index[arm][row] >= RGB_PER_ROW)
			RGB_Start_index[arm][row] = -parameter;//溢出归零
		if(row%2 != 0)//1、3列反转
			std::reverse(Arm_Inside_LED_Data[arm][row][0], Arm_Inside_LED_Data[arm][row][RGB_PER_ROW]);//此处Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-1]会导致灯条显示bug，暂未知原因
	}
	uint8_t return_num = RGB_PER_ROW+1-RGB_Start_index[arm][2]-parameter;//返回第三列尖端位置
	if(return_num > RGB_PER_ROW)
		return 0;
	else return return_num;
}
//传送带
static uint8_t Conveyer_Belt(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
	{
		//生成箭头
		if(row==1 || row==3)//1、3列前移一位
			RGB_Start_index[arm][row] = RGB_Start_index[arm][0];//此时row=0这列已经进入下一轮，RGB_Start_index[arm][0]已经加一，故无需在此加一
		else if(row==2)//2列前移两位
			RGB_Start_index[arm][row] = RGB_Start_index[arm][0] + 1;
		
		if(RGB_Start_index[arm][row] > parameter)
			RGB_Start_index[arm][row] -= parameter;
		for(uint8_t i=0; i<=((RGB_PER_ROW-RGB_Start_index[arm][row])/parameter); i++)//每个亮块的start RGB序号
		{
			if(RGB_Start_index[arm][row]+i*parameter < 0)
				if(i%2 == 0)
					memset(Arm_Inside_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
				else memset(Arm_Inside_LED_Data[arm][row][0], 0x00, (parameter+RGB_Start_index[arm][row])*3);
			else if(RGB_Start_index[arm][row]+i*parameter > RGB_PER_ROW-parameter)
				if(i%2 == 0)
					memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]+i*parameter], 0xff, (RGB_PER_ROW-RGB_Start_index[arm][row]-i*parameter)*3);
				else memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]+i*parameter], 0x00, (RGB_PER_ROW-RGB_Start_index[arm][row]-i*parameter)*3);
			else
				if(i%2 == 0)
					memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]+i*parameter], 0xff, parameter*3);
				else memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]+i*parameter], 0x00, parameter*3);
		}
		RGB_Start_index[arm][row]++;//累加
		if(RGB_Start_index[arm][row] >= parameter)
			RGB_Start_index[arm][row] = -parameter;//溢出归零
		if(row%2 != 0)//1、3列反转
			std::reverse(Arm_Inside_LED_Data[arm][row][0], Arm_Inside_LED_Data[arm][row][RGB_PER_ROW]);//此处Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-1]会导致灯条显示bug，暂未知原因
	}
	return 0;
}
//交叉进度条
static uint8_t Progress_Bar_0(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
	{
		memset(Arm_Inside_LED_Data[arm][row][0], 0xff, RGB_Start_index[arm][row]*3);
		if(RGB_Start_index[arm][row] < RGB_PER_ROW)
			RGB_Start_index[arm][row]++;//累加
	}
	return RGB_PER_ROW-RGB_Start_index[arm][2];
}
//同向进度条
static uint8_t Progress_Bar_1(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
	{
		memset(Arm_Inside_LED_Data[arm][row][0], 0xff, RGB_Start_index[arm][row]*3);
		if(RGB_Start_index[arm][row] < RGB_PER_ROW)
			RGB_Start_index[arm][row] += parameter;//累加
		if(row%2 != 0)//1、3列反转
			std::reverse(Arm_Inside_LED_Data[arm][row][0], Arm_Inside_LED_Data[arm][row][RGB_PER_ROW]);//此处Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-1]会导致灯条显示bug，暂未知原因
	}
	return RGB_PER_ROW-RGB_Start_index[arm][2];
}
//滴水进度条
static uint8_t Progress_Bar_2(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
	{
		//开头、结尾特殊处理
		if(RGB_Start_index[arm][row] < 0)
			memset(Arm_Inside_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
		else if(RGB_Start_index[arm][row] > RGB_PER_ROW-parameter)
			memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, (RGB_PER_ROW-RGB_Start_index[arm][row])*3);
		else
			memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, parameter*3);
		RGB_Start_index[arm][row]++;//累加
		if(RGB_Start_index[arm][row] > RGB_PER_ROW-RGB_Tail_num[arm][row]-parameter)
		{
			RGB_Start_index[arm][row] = -parameter;//溢出归零
			RGB_Tail_num[arm][row] += parameter;
		}
		memset(Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-RGB_Tail_num[arm][row]], 0xff, RGB_Tail_num[arm][row]*3);
		if(row%2 != 0)//1、3列反转
			std::reverse(Arm_Inside_LED_Data[arm][row][0], Arm_Inside_LED_Data[arm][row][RGB_PER_ROW]);//此处Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-1]会导致灯条显示bug，暂未知原因
		if(RGB_Tail_num[arm][row] >= RGB_PER_ROW)
			RGB_Tail_num[arm][row] = RGB_PER_ROW;
	}
	return RGB_PER_ROW-RGB_Tail_num[arm][3];
}
//俄罗斯方块
static uint8_t Tetris(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
	{
		//开头、结尾特殊处理
		if(RGB_Start_index[arm][row] < 0)
			memset(Arm_Inside_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
		else if(RGB_Start_index[arm][row] > RGB_PER_ROW-parameter)
			memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, (RGB_PER_ROW-RGB_Start_index[arm][row])*3);
		else
			memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, parameter*3);
		RGB_Start_index[arm][row]++;//累加
		if(RGB_Start_index[arm][row] > RGB_PER_ROW-RGB_Tail_num[arm][row]-parameter)
		{
			RGB_Start_index[arm][row] = -parameter;//溢出归零
			RGB_Tail_num[arm][row] += parameter;
		}
		memset(Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-RGB_Tail_num[arm][row]], 0xff, RGB_Tail_num[arm][row]*3);
		if(row%2 != 0)//1、3列反转
			std::reverse(Arm_Inside_LED_Data[arm][row][0], Arm_Inside_LED_Data[arm][row][RGB_PER_ROW]);//此处Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-1]会导致灯条显示bug，暂未知原因
		if(RGB_Tail_num[arm][row] >= RGB_PER_ROW)
			RGB_Tail_num[arm][row] = RGB_PER_ROW;
	}
	return RGB_PER_ROW-RGB_Tail_num[arm][3];
}

/** @brief  大符臂内部灯效设置
	* @param	[in]  arm	灯臂标号
	* @param	[in]  mode 模式
  *					This parameter can be one of the following values:
	*         @arg ALL_ON: 全亮(parameter无意义)(返回值无意义)
	*         @arg SLIDING_WINDOW: 滑动窗口(parameter为窗口大小)(返回滑块尖端距末端距离)
	*         @arg CONVEYER_BELT: 传送带(parameter为暗块/亮块宽度)(返回值无意义)
	*         @arg PROGRESS_BAR_0: 交叉进度条(parameter无意义)(返回进度信息)
	*         @arg PROGRESS_BAR_1: 同向进度条(parameter每次更新增加的长度)(返回进度信息)
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
uint8_t ARM_Inside_ligthting_effect(uint8_t arm, uint8_t mode, uint8_t parameter, uint8_t color)
{
	uint8_t return_data;
	//将每个RGB的亮灭配置信息放入Arm_Inside_LED_Data
	memset(Arm_Inside_LED_Data[arm], 0x00, sizeof(Arm_Inside_LED_Data[arm]));//全部清零
	switch(mode)
	{
		case ALL_ON:
			memset(Arm_Inside_LED_Data[arm], 0xff, sizeof(Arm_Inside_LED_Data[arm]));//全部置一
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
	//将颜色信息添加入Arm_Inside_LED_Data
	uint8_t color_set = 0;
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
		for(uint8_t led=0; led<RGB_PER_ROW; led++)
		{
			switch(color & 0xf8)
			{
				case 0:color_set = color;break;
				case RAND:color_set = rand()%6+1;break;
				case GRADATION:break;
				case RUNNING_WATER:break;
			}
			if(!(color_set & GREEN))
				Arm_Inside_LED_Data[arm][row][led][0] = 0x00;
			if(!(color_set & RED))
				Arm_Inside_LED_Data[arm][row][led][1] = 0x00;
			if(!(color_set & BLUE))
				Arm_Inside_LED_Data[arm][row][led][2] = 0x00;
		}
	return return_data;
}
/** @brief  大符臂外围灯效
	* @param	[in]  arm	灯臂标号
	* @param	[in]  mode 模式
  *					This parameter can be one of the following values:
	*         @arg UNSETLECTED: 未被选中，全灭
	*         @arg WAIT_HIT: 等待打击，矩形框亮，臂灯灭
	*         @arg HITTED: 已被打中，全亮
	*         @arg SUCCESS: 大符激活成功，流水效果
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
void ARM_Outside_ligthting_effect(uint8_t arm, uint8_t mode, uint8_t color)
{
	//将每个RGB的亮灭配置信息放入Arm_Inside_LED_Data
	memset(Arm_Outside_LED_Data[arm], 0x00, sizeof(Arm_Outside_LED_Data[arm]));//全部清零
	switch(mode)
	{
		case UNSETLECTED://未被选中，全灭
			break;
		case WAIT_HIT://等待打击，矩形框亮，臂灯灭
			memset(Arm_Outside_LED_Data[arm], 0xff, ARM_RECTANGLE_LENGTH*3);//矩形灯条全部置一
			break;
		case HITTED://已被打中，全亮
			memset(Arm_Outside_LED_Data[arm], 0xff, sizeof(Arm_Outside_LED_Data[arm]));//全部置一
			break;
		case SUCCESS://大符激活成功，流水效果
			memset(Arm_Outside_LED_Data[arm][0], 0xff, RGB_success_schedule[arm]*3);
			if(RGB_success_schedule[arm] < ARM_UTYPE_LENGTH+ARM_RECTANGLE_LENGTH)
				RGB_success_schedule[arm]++;//累加
			break;
	}
	//将颜色信息添加入Arm_Outside_LED_Data
	uint8_t color_set = 0;
	for(uint8_t led=0; led<ARM_UTYPE_LENGTH+ARM_RECTANGLE_LENGTH; led++)
	{
		switch(color & 0xf8)
		{
			case 0:color_set = color;break;
			case RAND:color_set = rand()%6+1;break;
			case GRADATION:break;
			case RUNNING_WATER:break;
		}
		if(!(color_set & GREEN))
			Arm_Outside_LED_Data[arm][0][led][0] = 0x00;
		if(!(color_set & RED))
			Arm_Outside_LED_Data[arm][0][led][1] = 0x00;
		if(!(color_set & BLUE))
			Arm_Outside_LED_Data[arm][0][led][2] = 0x00;
	}
}
/** @brief  初始化pwm
	* @param	[in]  arm	灯臂标号,每一位代表一个臂
	*/
void SMD_LED_PWM_Init(void)
{
	//设置各臂第一列第一个RGB中的绿色LED pwm脉冲占空比，
	if(Arm_Inside_LED_Data[0][0][Inside_LED_index[0]/3][Inside_LED_index[0]%3] == 0xff)
		ARM0_PULSE = LOGIC_ONE_PULSE;
	else
		ARM0_PULSE = LOGIC_ZERO_PULSE;

	if(Arm_Inside_LED_Data[1][0][Inside_LED_index[1]/3][Inside_LED_index[1]%3] == 0xff)
		ARM1_PULSE = LOGIC_ONE_PULSE;
	else
		ARM1_PULSE = LOGIC_ZERO_PULSE;

	if(Arm_Inside_LED_Data[2][0][Inside_LED_index[2]/3][Inside_LED_index[2]%3] == 0xff)
		ARM2_PULSE = LOGIC_ONE_PULSE;
	else
		ARM2_PULSE = LOGIC_ZERO_PULSE;
	//开启pwm中断
	__HAL_TIM_ENABLE_IT(ARM_TIM,TIM_IT_UPDATE);
}

void SMD_INSIDE_LED_IT(void)//内部灯阵中断处理函数
{
	for(uint8_t i=0; i<ARM_PER_BOARD; i++)
	{
		if(Arm_Inside_LED_Data[i][Inside_row_index[i]][Inside_RGB_index[i]][Inside_LED_index[i]] == 0xff)
			switch(i)
			{
				case 0:ARM0_PULSE = LOGIC_ONE_PULSE;break;
				case 1:ARM1_PULSE = LOGIC_ONE_PULSE;break;
				case 2:ARM2_PULSE = LOGIC_ONE_PULSE;break;
			}
		else
			switch(i)
			{
				case 0:ARM0_PULSE = LOGIC_ZERO_PULSE;break;
				case 1:ARM1_PULSE = LOGIC_ZERO_PULSE;break;
				case 2:ARM2_PULSE = LOGIC_ZERO_PULSE;break;
			}
		Inside_bit_index[i]++;
		if(Inside_bit_index[i] == 8)//一个LED的8位数据遍历完
		{
			Inside_bit_index[i] = 0;//数据位指针归零
			Inside_LED_index[i]++;//下一个LED
			if(Inside_LED_index[i] == 3)
			{
				Inside_LED_index[i] = 0;//LED指针清零
				Inside_RGB_index[i]++;//下一个RGB
				if(Inside_RGB_index[i] == RGB_PER_ROW)
				{
					Inside_RGB_index[i] = 0;//RGB指针清零
					Inside_row_index[i]++;//下一列
					if(Inside_row_index[i] == ROW_PER_ARM)
					{
						if(!((i+1)<ARM_PER_BOARD))//不满足下次循环表示是需要遍历的最后一个灯臂
						{
							__HAL_TIM_DISABLE_IT(ARM_TIM,TIM_IT_UPDATE);//关中断
							ARM0_PULSE = 0;
							ARM1_PULSE = 0;
							ARM2_PULSE = 0;
						}
						Inside_row_index[i] = 0;//列指针清零
						Inside_bit_index[i] = 1;//当遍历完一个臂上所有RGB的所有LED的所有位，下次进中断设置的应是第二个LED的占空比，故此处为1
					}
				}
			}
		}
	}
}
void SMD_OUTSIDE_LED_IT(void)//外部灯条中断处理函数
{
	for(uint8_t i=0; i<ARM_PER_BOARD; i++)
	{
		if(Arm_Outside_LED_Data[i][Outside_row_index[i]][Outside_RGB_index[i]][Outside_LED_index[i]] == 0xff)
			switch(i)
			{
				case 0:ARM0_PULSE = LOGIC_ONE_PULSE;break;
				case 1:ARM1_PULSE = LOGIC_ONE_PULSE;break;
				case 2:ARM2_PULSE = LOGIC_ONE_PULSE;break;
			}
		else
			switch(i)
			{
				case 0:ARM0_PULSE = LOGIC_ZERO_PULSE;break;
				case 1:ARM1_PULSE = LOGIC_ZERO_PULSE;break;
				case 2:ARM2_PULSE = LOGIC_ZERO_PULSE;break;
			}
		Outside_bit_index[i]++;
		if(Outside_bit_index[i] == 8)//一个LED的8位数据遍历完
		{
			Outside_bit_index[i] = 0;//数据位指针归零
			Outside_LED_index[i]++;//下一个LED
			if(Outside_LED_index[i] == 3)
			{
				Outside_LED_index[i] = 0;//LED指针清零
				Outside_RGB_index[i]++;//下一个RGB
				if(Outside_RGB_index[i] == ARM_UTYPE_LENGTH+ARM_RECTANGLE_LENGTH)
				{
					Outside_RGB_index[i] = 0;//RGB指针清零
					Outside_row_index[i]++;//下一列
					if(Outside_row_index[i] == 1)
					{
						if(!((i+1)<ARM_PER_BOARD))//不满足下次循环表示是需要遍历的最后一个灯臂
						{
							__HAL_TIM_DISABLE_IT(ARM_TIM,TIM_IT_UPDATE);//关中断
							ARM0_PULSE = 0;
							ARM1_PULSE = 0;
							ARM2_PULSE = 0;
						}
						Outside_row_index[i] = 0;//列指针清零
						Outside_bit_index[i] = 1;//当遍历完一个臂上所有RGB的所有LED的所有位，下次进中断设置的应是第二个LED的占空比，故此处为1
					}
				}
			}
		}
	}
}
//void SMD_OUTSIDE_LED_IT(void)//外部灯条中断处理函数
//{
//	for(uint8_t arm=0; arm<ARM_PER_BOARD; arm++)
//	{
//		if(Arm_Outside_LED_Data[arm][Outside_RGB_index[arm]][Outside_LED_index[arm]] == 0xff)
//			switch(arm)
//			{
//				case 0:ARM0_PULSE = LOGIC_ONE_PULSE;break;
//				case 1:ARM1_PULSE = LOGIC_ONE_PULSE;break;
//				case 2:ARM2_PULSE = LOGIC_ONE_PULSE;break;
//			}
//		else
//			switch(arm)
//			{
//				case 0:ARM0_PULSE = LOGIC_ZERO_PULSE;break;
//				case 1:ARM1_PULSE = LOGIC_ZERO_PULSE;break;
//				case 2:ARM2_PULSE = LOGIC_ZERO_PULSE;break;
//			}
//		Outside_bit_index[arm]++;
//		if(Outside_bit_index[arm] == 8)//一个LED的8位数据遍历完
//		{
//			Outside_bit_index[arm] = 0;//数据位指针归零
//			Outside_LED_index[arm]++;//下一个LED
//			if(Outside_LED_index[arm] == 3)
//			{
//				Outside_LED_index[arm] = 0;//LED指针清零
//				Outside_RGB_index[arm]++;//下一个RGB
//				if(Outside_RGB_index[arm] == ARM_UTYPE_LENGTH+ARM_RECTANGLE_LENGTH)
//				{
//					if(!((arm+1)<ARM_PER_BOARD))//不满足下次循环表示是需要遍历的最后一个灯臂
//					{
//						__HAL_TIM_DISABLE_IT(ARM_TIM,TIM_IT_UPDATE);//关中断
//						ARM0_PULSE = 0;
//						ARM1_PULSE = 0;
//						ARM2_PULSE = 0;
//					}
//					Outside_bit_index[arm] = 1;//当遍历完一个臂上所有RGB的所有LED的所有位，下次进中断设置的应是第二个LED的占空比，故此处为1
//				}
//			}
//		}
//	}
//}

