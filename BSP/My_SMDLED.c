/** 
* @brief    SMD��LED�弶֧�ְ�
* @details  SMD_INSIDE_LED_IT()��Ҫ�ŵ�ʱ���жϻص�������
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
#include "define_all.h"

#define ARM_PER_BOARD 3//ÿ�����������Ʊ۸���
#define ROW_PER_ARM 5//��������
#define RGB_PER_ROW 64//����RGB��
#define ARM_UTYPE_LENGTH 100//�������Χ��������
#define ARM_RECTANGLE_LENGTH 123//���ο��������

uint8_t Arm_Outside_LED_Data[ARM_PER_BOARD][1][ARM_UTYPE_LENGTH+ARM_RECTANGLE_LENGTH][3] = {0xff};//Arm_Outside_LED_Data[�糵�����][����RGB��/2][��RGB LED��]
uint8_t Outside_row_index[ARM_PER_BOARD] = {0};//5���۸��Ե���ָ��(0)
uint8_t Outside_RGB_index[ARM_PER_BOARD] = {0};//5���۸��Ե�RGBָ��(0~RGB_PER_ROW-1)
uint8_t Outside_LED_index[ARM_PER_BOARD] = {0};//5���۸��Ե�LEDָ��(0~2)
uint8_t Outside_bit_index[ARM_PER_BOARD] = {1};//5���۸��Ե�����λָ��(0-7),��0���ź���SMD_LED_Color_Set�����ã��ж��дӵڶ����źſ�ʼ����
uint8_t Arm_Inside_LED_Data[ARM_PER_BOARD][ROW_PER_ARM][RGB_PER_ROW][3] = {0xff};//Arm_Inside_LED_Data[�糵�����][����RGB����][���۵���RGB��][��RGB LED��]
uint8_t Inside_row_index[ARM_PER_BOARD] = {0};//5���۸��Ե���ָ��(0~4)
uint8_t Inside_RGB_index[ARM_PER_BOARD] = {0};//5���۸��Ե�RGBָ��(0~RGB_PER_ROW-1)
uint8_t Inside_LED_index[ARM_PER_BOARD] = {0};//5���۸��Ե�LEDָ��(0~2)
uint8_t Inside_bit_index[ARM_PER_BOARD] = {1};//5���۸��Ե�����λָ��(0-7),��0���ź���SMD_LED_Color_Set�����ã��ж��дӵڶ����źſ�ʼ����

//ARM_Inside_ligthting_effectר������
int8_t RGB_Start_index[5][ROW_PER_ARM] = {0};//���浱ǰ�����¸����۸��е���ʼ��RGB���
uint8_t RGB_Tail_num[5][ROW_PER_ARM] = {0};//���浱ǰ�����¸����۸��е�β���ۼ��ķ���RGB����(��ˮ������������˹����ר��)
uint8_t RGB_success_schedule[5] = {0};//����ɹ���Ե����������������Ϣ

//��������
static uint8_t Sliding_Window(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
	{
		//���ɼ�ͷ
		if(row==1 || row==3)//1��3��ǰ��һλ
			RGB_Start_index[arm][row] = RGB_Start_index[arm][0];//��ʱrow=0�����Ѿ�������һ�֣�RGB_Start_index[arm][0]�Ѿ���һ���������ڴ˼�һ
		else if(row==2)//2��ǰ����λ
			RGB_Start_index[arm][row] = RGB_Start_index[arm][0] + 1;
		//��ͷ����β���⴦��
		if(RGB_Start_index[arm][row] < 0)
			memset(Arm_Inside_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
		else if(RGB_Start_index[arm][row] > RGB_PER_ROW-parameter)
			memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, (RGB_PER_ROW-RGB_Start_index[arm][row])*3);
		else
			memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, parameter*3);
		RGB_Start_index[arm][row]++;//�ۼ�
		if(RGB_Start_index[arm][row] >= RGB_PER_ROW)
			RGB_Start_index[arm][row] = -parameter;//�������
		if(row%2 != 0)//1��3�з�ת
			std::reverse(Arm_Inside_LED_Data[arm][row][0], Arm_Inside_LED_Data[arm][row][RGB_PER_ROW]);//�˴�Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-1]�ᵼ�µ�����ʾbug����δ֪ԭ��
	}
	uint8_t return_num = RGB_PER_ROW+1-RGB_Start_index[arm][2]-parameter;//���ص����м��λ��
	if(return_num > RGB_PER_ROW)
		return 0;
	else return return_num;
}
//���ʹ�
static uint8_t Conveyer_Belt(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
	{
		//���ɼ�ͷ
		if(row==1 || row==3)//1��3��ǰ��һλ
			RGB_Start_index[arm][row] = RGB_Start_index[arm][0];//��ʱrow=0�����Ѿ�������һ�֣�RGB_Start_index[arm][0]�Ѿ���һ���������ڴ˼�һ
		else if(row==2)//2��ǰ����λ
			RGB_Start_index[arm][row] = RGB_Start_index[arm][0] + 1;
		
		if(RGB_Start_index[arm][row] > parameter)
			RGB_Start_index[arm][row] -= parameter;
		for(uint8_t i=0; i<=((RGB_PER_ROW-RGB_Start_index[arm][row])/parameter); i++)//ÿ�������start RGB���
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
		RGB_Start_index[arm][row]++;//�ۼ�
		if(RGB_Start_index[arm][row] >= parameter)
			RGB_Start_index[arm][row] = -parameter;//�������
		if(row%2 != 0)//1��3�з�ת
			std::reverse(Arm_Inside_LED_Data[arm][row][0], Arm_Inside_LED_Data[arm][row][RGB_PER_ROW]);//�˴�Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-1]�ᵼ�µ�����ʾbug����δ֪ԭ��
	}
	return 0;
}
//���������
static uint8_t Progress_Bar_0(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
	{
		memset(Arm_Inside_LED_Data[arm][row][0], 0xff, RGB_Start_index[arm][row]*3);
		if(RGB_Start_index[arm][row] < RGB_PER_ROW)
			RGB_Start_index[arm][row]++;//�ۼ�
	}
	return RGB_PER_ROW-RGB_Start_index[arm][2];
}
//ͬ�������
static uint8_t Progress_Bar_1(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
	{
		memset(Arm_Inside_LED_Data[arm][row][0], 0xff, RGB_Start_index[arm][row]*3);
		if(RGB_Start_index[arm][row] < RGB_PER_ROW)
			RGB_Start_index[arm][row] += parameter;//�ۼ�
		if(row%2 != 0)//1��3�з�ת
			std::reverse(Arm_Inside_LED_Data[arm][row][0], Arm_Inside_LED_Data[arm][row][RGB_PER_ROW]);//�˴�Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-1]�ᵼ�µ�����ʾbug����δ֪ԭ��
	}
	return RGB_PER_ROW-RGB_Start_index[arm][2];
}
//��ˮ������
static uint8_t Progress_Bar_2(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
	{
		//��ͷ����β���⴦��
		if(RGB_Start_index[arm][row] < 0)
			memset(Arm_Inside_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
		else if(RGB_Start_index[arm][row] > RGB_PER_ROW-parameter)
			memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, (RGB_PER_ROW-RGB_Start_index[arm][row])*3);
		else
			memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, parameter*3);
		RGB_Start_index[arm][row]++;//�ۼ�
		if(RGB_Start_index[arm][row] > RGB_PER_ROW-RGB_Tail_num[arm][row]-parameter)
		{
			RGB_Start_index[arm][row] = -parameter;//�������
			RGB_Tail_num[arm][row] += parameter;
		}
		memset(Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-RGB_Tail_num[arm][row]], 0xff, RGB_Tail_num[arm][row]*3);
		if(row%2 != 0)//1��3�з�ת
			std::reverse(Arm_Inside_LED_Data[arm][row][0], Arm_Inside_LED_Data[arm][row][RGB_PER_ROW]);//�˴�Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-1]�ᵼ�µ�����ʾbug����δ֪ԭ��
		if(RGB_Tail_num[arm][row] >= RGB_PER_ROW)
			RGB_Tail_num[arm][row] = RGB_PER_ROW;
	}
	return RGB_PER_ROW-RGB_Tail_num[arm][3];
}
//����˹����
static uint8_t Tetris(uint8_t arm, uint8_t parameter)
{
	for(uint8_t row=0; row<ROW_PER_ARM; row++)
	{
		//��ͷ����β���⴦��
		if(RGB_Start_index[arm][row] < 0)
			memset(Arm_Inside_LED_Data[arm][row][0], 0xff, (parameter+RGB_Start_index[arm][row])*3);
		else if(RGB_Start_index[arm][row] > RGB_PER_ROW-parameter)
			memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, (RGB_PER_ROW-RGB_Start_index[arm][row])*3);
		else
			memset(Arm_Inside_LED_Data[arm][row][RGB_Start_index[arm][row]], 0xff, parameter*3);
		RGB_Start_index[arm][row]++;//�ۼ�
		if(RGB_Start_index[arm][row] > RGB_PER_ROW-RGB_Tail_num[arm][row]-parameter)
		{
			RGB_Start_index[arm][row] = -parameter;//�������
			RGB_Tail_num[arm][row] += parameter;
		}
		memset(Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-RGB_Tail_num[arm][row]], 0xff, RGB_Tail_num[arm][row]*3);
		if(row%2 != 0)//1��3�з�ת
			std::reverse(Arm_Inside_LED_Data[arm][row][0], Arm_Inside_LED_Data[arm][row][RGB_PER_ROW]);//�˴�Arm_Inside_LED_Data[arm][row][RGB_PER_ROW-1]�ᵼ�µ�����ʾbug����δ֪ԭ��
		if(RGB_Tail_num[arm][row] >= RGB_PER_ROW)
			RGB_Tail_num[arm][row] = RGB_PER_ROW;
	}
	return RGB_PER_ROW-RGB_Tail_num[arm][3];
}

/** @brief  ������ڲ���Ч����
	* @param	[in]  arm	�Ʊ۱��
	* @param	[in]  mode ģʽ
  *					This parameter can be one of the following values:
	*         @arg ALL_ON: ȫ��(parameter������)(����ֵ������)
	*         @arg SLIDING_WINDOW: ��������(parameterΪ���ڴ�С)(���ػ����˾�ĩ�˾���)
	*         @arg CONVEYER_BELT: ���ʹ�(parameterΪ����/������)(����ֵ������)
	*         @arg PROGRESS_BAR_0: ���������(parameter������)(���ؽ�����Ϣ)
	*         @arg PROGRESS_BAR_1: ͬ�������(parameterÿ�θ������ӵĳ���)(���ؽ�����Ϣ)
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
uint8_t ARM_Inside_ligthting_effect(uint8_t arm, uint8_t mode, uint8_t parameter, uint8_t color)
{
	uint8_t return_data;
	//��ÿ��RGB������������Ϣ����Arm_Inside_LED_Data
	memset(Arm_Inside_LED_Data[arm], 0x00, sizeof(Arm_Inside_LED_Data[arm]));//ȫ������
	switch(mode)
	{
		case ALL_ON:
			memset(Arm_Inside_LED_Data[arm], 0xff, sizeof(Arm_Inside_LED_Data[arm]));//ȫ����һ
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
	//����ɫ��Ϣ�����Arm_Inside_LED_Data
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
/** @brief  �������Χ��Ч
	* @param	[in]  arm	�Ʊ۱��
	* @param	[in]  mode ģʽ
  *					This parameter can be one of the following values:
	*         @arg UNSETLECTED: δ��ѡ�У�ȫ��
	*         @arg WAIT_HIT: �ȴ���������ο������۵���
	*         @arg HITTED: �ѱ����У�ȫ��
	*         @arg SUCCESS: �������ɹ�����ˮЧ��
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
void ARM_Outside_ligthting_effect(uint8_t arm, uint8_t mode, uint8_t color)
{
	//��ÿ��RGB������������Ϣ����Arm_Inside_LED_Data
	memset(Arm_Outside_LED_Data[arm], 0x00, sizeof(Arm_Outside_LED_Data[arm]));//ȫ������
	switch(mode)
	{
		case UNSETLECTED://δ��ѡ�У�ȫ��
			break;
		case WAIT_HIT://�ȴ���������ο������۵���
			memset(Arm_Outside_LED_Data[arm], 0xff, ARM_RECTANGLE_LENGTH*3);//���ε���ȫ����һ
			break;
		case HITTED://�ѱ����У�ȫ��
			memset(Arm_Outside_LED_Data[arm], 0xff, sizeof(Arm_Outside_LED_Data[arm]));//ȫ����һ
			break;
		case SUCCESS://�������ɹ�����ˮЧ��
			memset(Arm_Outside_LED_Data[arm][0], 0xff, RGB_success_schedule[arm]*3);
			if(RGB_success_schedule[arm] < ARM_UTYPE_LENGTH+ARM_RECTANGLE_LENGTH)
				RGB_success_schedule[arm]++;//�ۼ�
			break;
	}
	//����ɫ��Ϣ�����Arm_Outside_LED_Data
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
/** @brief  ��ʼ��pwm
	* @param	[in]  arm	�Ʊ۱��,ÿһλ����һ����
	*/
void SMD_LED_PWM_Init(void)
{
	//���ø��۵�һ�е�һ��RGB�е���ɫLED pwm����ռ�ձȣ�
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
	//����pwm�ж�
	__HAL_TIM_ENABLE_IT(ARM_TIM,TIM_IT_UPDATE);
}

void SMD_INSIDE_LED_IT(void)//�ڲ������жϴ�����
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
		if(Inside_bit_index[i] == 8)//һ��LED��8λ���ݱ�����
		{
			Inside_bit_index[i] = 0;//����λָ�����
			Inside_LED_index[i]++;//��һ��LED
			if(Inside_LED_index[i] == 3)
			{
				Inside_LED_index[i] = 0;//LEDָ������
				Inside_RGB_index[i]++;//��һ��RGB
				if(Inside_RGB_index[i] == RGB_PER_ROW)
				{
					Inside_RGB_index[i] = 0;//RGBָ������
					Inside_row_index[i]++;//��һ��
					if(Inside_row_index[i] == ROW_PER_ARM)
					{
						if(!((i+1)<ARM_PER_BOARD))//�������´�ѭ����ʾ����Ҫ���������һ���Ʊ�
						{
							__HAL_TIM_DISABLE_IT(ARM_TIM,TIM_IT_UPDATE);//���ж�
							ARM0_PULSE = 0;
							ARM1_PULSE = 0;
							ARM2_PULSE = 0;
						}
						Inside_row_index[i] = 0;//��ָ������
						Inside_bit_index[i] = 1;//��������һ����������RGB������LED������λ���´ν��ж����õ�Ӧ�ǵڶ���LED��ռ�ձȣ��ʴ˴�Ϊ1
					}
				}
			}
		}
	}
}
void SMD_OUTSIDE_LED_IT(void)//�ⲿ�����жϴ�����
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
		if(Outside_bit_index[i] == 8)//һ��LED��8λ���ݱ�����
		{
			Outside_bit_index[i] = 0;//����λָ�����
			Outside_LED_index[i]++;//��һ��LED
			if(Outside_LED_index[i] == 3)
			{
				Outside_LED_index[i] = 0;//LEDָ������
				Outside_RGB_index[i]++;//��һ��RGB
				if(Outside_RGB_index[i] == ARM_UTYPE_LENGTH+ARM_RECTANGLE_LENGTH)
				{
					Outside_RGB_index[i] = 0;//RGBָ������
					Outside_row_index[i]++;//��һ��
					if(Outside_row_index[i] == 1)
					{
						if(!((i+1)<ARM_PER_BOARD))//�������´�ѭ����ʾ����Ҫ���������һ���Ʊ�
						{
							__HAL_TIM_DISABLE_IT(ARM_TIM,TIM_IT_UPDATE);//���ж�
							ARM0_PULSE = 0;
							ARM1_PULSE = 0;
							ARM2_PULSE = 0;
						}
						Outside_row_index[i] = 0;//��ָ������
						Outside_bit_index[i] = 1;//��������һ����������RGB������LED������λ���´ν��ж����õ�Ӧ�ǵڶ���LED��ռ�ձȣ��ʴ˴�Ϊ1
					}
				}
			}
		}
	}
}
//void SMD_OUTSIDE_LED_IT(void)//�ⲿ�����жϴ�����
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
//		if(Outside_bit_index[arm] == 8)//һ��LED��8λ���ݱ�����
//		{
//			Outside_bit_index[arm] = 0;//����λָ�����
//			Outside_LED_index[arm]++;//��һ��LED
//			if(Outside_LED_index[arm] == 3)
//			{
//				Outside_LED_index[arm] = 0;//LEDָ������
//				Outside_RGB_index[arm]++;//��һ��RGB
//				if(Outside_RGB_index[arm] == ARM_UTYPE_LENGTH+ARM_RECTANGLE_LENGTH)
//				{
//					if(!((arm+1)<ARM_PER_BOARD))//�������´�ѭ����ʾ����Ҫ���������һ���Ʊ�
//					{
//						__HAL_TIM_DISABLE_IT(ARM_TIM,TIM_IT_UPDATE);//���ж�
//						ARM0_PULSE = 0;
//						ARM1_PULSE = 0;
//						ARM2_PULSE = 0;
//					}
//					Outside_bit_index[arm] = 1;//��������һ����������RGB������LED������λ���´ν��ж����õ�Ӧ�ǵڶ���LED��ռ�ձȣ��ʴ˴�Ϊ1
//				}
//			}
//		}
//	}
//}

