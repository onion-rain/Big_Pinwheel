/** 
* @brief    SMD��LED�弶֧�ְ�
* @details  
* @author   Onion rain
* @date     2019.3.4
* @version  1.0
* @par Copyright (c):  Onion rain
* @par ��־
*/

#include "My_SMDLED.h"
#include "string.h"
#include "tim.h"
#include <algorithm>
#include <stdlib.h>

int tmp = 0;

#define MAX_RGB_NUM 72//���г���

uint8_t Arm_LED_Data[5][5][MAX_RGB_NUM][3] = {0xff};//Arm_LED_Data[�糵�����][����RGB����][���۵���RGB��][��RGB LED��]
uint8_t row_index[5] = {0};//5���۸��Ե���ָ��(0~4)
uint8_t RGB_index[5] = {0};//5���۸��Ե�RGBָ��(0~MAX_RGB_NUM-1)
uint8_t LED_index[5] = {0};//5���۸��Ե�LEDָ��(0~2)
uint8_t bit_index[5] = {1};//5���۸��Ե�����λָ��(0-7),��0���ź���SMD_LED_Color_Set�����ã��ж��дӵڶ����źſ�ʼ����

//SMD_LED_Running_Water_Effect_Configurationר������
int8_t RGB_Start_index[5][5] = {0};//���浱ǰ�����¸����۸��е���ʼ��RGB���
uint8_t RGB_Tail_num[5][5] = {0};//���浱ǰ�����¸����۸��е�β���ۼ��ķ���RGB����(��ˮ������������˹����ר��)

//��������
static uint8_t Sliding_Window(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<5; row++)
	{
		//���ɼ�ͷ
		if(row==1 || row==3)//1��3��ǰ��һλ
			RGB_Start_index[arm][row] = RGB_Start_index[arm][0];//��ʱrow=0�����Ѿ�������һ�֣�RGB_Start_index[arm][0]�Ѿ���һ���������ڴ˼�һ
		else if(row==2)//2��ǰ����λ
			RGB_Start_index[arm][row] = RGB_Start_index[arm][0] + 1;
		//��ͷ����β���⴦��
		if(RGB_Start_index[arm][row] < 0)
			memset(Arm_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
		else if(RGB_Start_index[arm][row] > MAX_RGB_NUM-parameter)
			memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, (MAX_RGB_NUM-RGB_Start_index[arm][row])*3);
		else
			memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, parameter*3);
		RGB_Start_index[arm][row]++;//�ۼ�
		if(RGB_Start_index[arm][row] >= MAX_RGB_NUM)
			RGB_Start_index[arm][row] = -parameter;//�������
		if(row%2 != 0)//1��3�з�ת
			std::reverse(Arm_LED_Data[arm][row][0], Arm_LED_Data[arm][row][MAX_RGB_NUM]);//�˴�Arm_LED_Data[arm][row][MAX_RGB_NUM-1]�ᵼ�µ�����ʾbug����δ֪ԭ��
	}
	uint8_t return_num = MAX_RGB_NUM+1-RGB_Start_index[arm][2]-parameter;//���ص����м��λ��
	if(return_num > MAX_RGB_NUM)
		return 0;
	else return return_num;
}
//���ʹ�
static uint8_t Conveyer_Belt(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<5; row++)
	{
		//���ɼ�ͷ
		if(row==1 || row==3)//1��3��ǰ��һλ
			RGB_Start_index[arm][row] = RGB_Start_index[arm][0];//��ʱrow=0�����Ѿ�������һ�֣�RGB_Start_index[arm][0]�Ѿ���һ���������ڴ˼�һ
		else if(row==2)//2��ǰ����λ
			RGB_Start_index[arm][row] = RGB_Start_index[arm][0] + 1;
		
		if(RGB_Start_index[arm][row] > parameter)
			RGB_Start_index[arm][row] -= parameter;
		for(uint8_t i=0; i<=((MAX_RGB_NUM-RGB_Start_index[arm][row])/parameter); i++)//ÿ�������start RGB���
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
		RGB_Start_index[arm][row]++;//�ۼ�
		if(RGB_Start_index[arm][row] >= parameter)
			RGB_Start_index[arm][row] = -parameter;//�������
		if(row%2 != 0)//1��3�з�ת
			std::reverse(Arm_LED_Data[arm][row][0], Arm_LED_Data[arm][row][MAX_RGB_NUM]);//�˴�Arm_LED_Data[arm][row][MAX_RGB_NUM-1]�ᵼ�µ�����ʾbug����δ֪ԭ��
	}
	return 0;
}
//���������
static uint8_t Progress_Bar_0(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<5; row++)
	{
		memset(Arm_LED_Data[arm][row][0], 0xff, RGB_Start_index[arm][row]*3);
		if(RGB_Start_index[arm][row] < MAX_RGB_NUM)
			RGB_Start_index[arm][row]++;//�ۼ�
	}
	return MAX_RGB_NUM-RGB_Start_index[arm][2];
}
//ͬ�������
static uint8_t Progress_Bar_1(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<5; row++)
	{
		memset(Arm_LED_Data[arm][row][0], 0xff, RGB_Start_index[arm][row]*3);
		if(RGB_Start_index[arm][row] < MAX_RGB_NUM)
			RGB_Start_index[arm][row]++;//�ۼ�
		if(row%2 != 0)//1��3�з�ת
			std::reverse(Arm_LED_Data[arm][row][0], Arm_LED_Data[arm][row][MAX_RGB_NUM]);//�˴�Arm_LED_Data[arm][row][MAX_RGB_NUM-1]�ᵼ�µ�����ʾbug����δ֪ԭ��
	}
	return MAX_RGB_NUM-RGB_Start_index[arm][2];
}
//��ˮ������
static uint8_t Progress_Bar_2(uint8_t arm, uint8_t parameter)//�ú����������ָ��ָ�ɣ�����������һ��ʱ�䲻�����ᵼ������tim8���ж�����߽�errorhandle
{
	for(uint8_t row=0; row<5; row++)
	{
		//��ͷ����β���⴦��
		if(RGB_Start_index[arm][row] < 0)
			memset(Arm_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
		else if(RGB_Start_index[arm][row] > MAX_RGB_NUM-parameter)
			memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, (MAX_RGB_NUM-RGB_Start_index[arm][row])*3);
		else
			memset(Arm_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, parameter*3);
		RGB_Start_index[arm][row]++;//�ۼ�
		if(RGB_Start_index[arm][row] > MAX_RGB_NUM-RGB_Tail_num[arm][row]-parameter && RGB_Tail_num[arm][row] < MAX_RGB_NUM)
		{
			RGB_Start_index[arm][row] = -parameter;//�������
			RGB_Tail_num[arm][row] += parameter;
		}
		if(RGB_Tail_num[arm][row] != 0)
			memset(Arm_LED_Data[arm][row][MAX_RGB_NUM-RGB_Tail_num[arm][row]], 0xff, RGB_Tail_num[arm][row]*3);
		if(row%2 != 0)//1��3�з�ת
			std::reverse(Arm_LED_Data[arm][row][0], Arm_LED_Data[arm][row][MAX_RGB_NUM]);//�˴�Arm_LED_Data[arm][row][MAX_RGB_NUM-1]�ᵼ�µ�����ʾbug����δ֪ԭ��
		if(RGB_Tail_num[arm][row] >= MAX_RGB_NUM)
			RGB_Tail_num[arm][row] = MAX_RGB_NUM;
	}
	return MAX_RGB_NUM-RGB_Tail_num[arm][3];
}
//����˹����
static uint8_t Tetris(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<5; row++)
	{
		
	}
	return 0;
}

/** @brief  ������ˮ��Ч����
	* @param	[in]  arm	�Ʊ۱��
	* @param	[in]  mode ģʽ
  *					This parameter can be one of the following values:
	*         @arg ALL_ON: ȫ��(parameter������)(����ֵ������)
	*         @arg SLIDING_WINDOW: ��������(parameterΪ���ڴ�С)(���ػ����˾�ĩ�˾���)
	*         @arg CONVEYER_BELT: ���ʹ�(parameterΪ����/������)(����ֵ������)
	*         @arg PROGRESS_BAR_0: ���������(parameter������)(���ؽ�����Ϣ)
	*         @arg PROGRESS_BAR_1: ͬ�������(parameter������)(���ؽ�����Ϣ)
	*         @arg PROGRESS_BAR_3: ��ˮ������(parameterΪˮ�γ���)(���ؽ�����Ϣ)
	*         @arg TETRIS: ����˹����(parameter�����С)(�����Ƿ���Ϸʧ��,0����/1ʧ��)
	* @param	[in]  parameter ����
	* @param	[in]  color ��ʾ����ɫ
  *					This parameter can be one of the following values:
	*         @arg GREEN: ��ɫ��ɫ
	*         @arg RED: 	��ɫ��ɫ
	*         @arg BLUE:  ��ɫ��ɫ(��ɫ�����|���ɴ���)
	*         @arg RAND:	���ɫ
	*         @arg RUNNING_WATER:	��ˮ��Ч
	* @details	���ڵ��øú����Դﵽ��ˮЧ��
	* @retval ��ͬmode���岻ͬ
	*/
uint8_t SMD_LED_Running_Water_Effect_Configuration(uint8_t arm, uint8_t mode, uint8_t parameter, uint8_t color)
{
	uint8_t return_data;
	//��ÿ��RGB������������Ϣ����Arm_LED_Data
	memset(Arm_LED_Data[arm], 0x00, sizeof(Arm_LED_Data[arm]));//ȫ������
	switch(mode)
	{
		case ALL_ON:
			memset(Arm_LED_Data[arm], 0xff, sizeof(Arm_LED_Data[arm]));//ȫ����һ
			return_data = 0;
			break;
		case SLIDING_WINDOW://��������
			return_data = Sliding_Window(arm, parameter);
			break;
		case CONVEYER_BELT://���ʹ�
			return_data = Conveyer_Belt(arm, parameter);
			break;
		case PROGRESS_BAR_0://���������
			return_data = Progress_Bar_0(arm, parameter);
			break;
		case PROGRESS_BAR_1://ͬ�������
			return_data = Progress_Bar_1(arm, parameter);
			break;
		case PROGRESS_BAR_2://��ˮ������
			return_data = Progress_Bar_2(arm, parameter);
			break;
		case TETRIS://����˹����
			return_data = Tetris(arm, parameter);
			break;
	}
	//����ɫ��Ϣ�����Arm_LED_Data
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
	//���õ�һ�е�һ��RGB�е���ɫLED pwm����ռ�ձȣ�
	if(Arm_LED_Data[arm][0][LED_index[1]/3][LED_index[1]%3] == 0xff)
		ARM1_PULSE = LOGIC_ONE_PULSE;
	else ARM1_PULSE = LOGIC_ZERO_PULSE;
	//����pwm�ж�
	__HAL_TIM_ENABLE_IT(&htim2,TIM_IT_UPDATE);
	return return_data;
}

void SMD_LED_TIM2_IT(void)
{
	if(Arm_LED_Data[1][row_index[1]][RGB_index[1]][LED_index[1]] == 0xff)
		ARM1_PULSE = LOGIC_ONE_PULSE;
	else ARM1_PULSE = LOGIC_ZERO_PULSE;
	bit_index[1]++;
	if(bit_index[1] == 8)//һ��LED��8λ���ݱ�����
	{
		bit_index[1] = 0;//����λָ�����
		LED_index[1]++;//��һ��LED
		if(LED_index[1] == 3)
		{
			LED_index[1] = 0;//LEDָ������
			RGB_index[1]++;//��һ��RGB
			if(RGB_index[1] == MAX_RGB_NUM)
			{
				RGB_index[1] = 0;//RGBָ������
				row_index[1]++;//��һ��
				if(row_index[1] == 5)
				{
					__HAL_TIM_DISABLE_IT(&htim2,TIM_IT_UPDATE);//���ж�
					ARM1_PULSE = 0;//IO����
					row_index[1] = 0;//��ָ������
					bit_index[1] = 1;//��������һ����������RGB������LED������λ���´ν��ж����õ�Ӧ�ǵڶ���LED��ռ�ձȣ��ʴ˴�Ϊ1
				}
			}
		}
	}
}





