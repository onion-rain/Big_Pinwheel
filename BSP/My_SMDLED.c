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

#define MAX_RGB_NUM 72//���г���

uint8_t Arm_LED_Data[5][5][MAX_RGB_NUM][3] = {0xff};//Arm_LED_Data[�糵�����][����RGB����][���۵���RGB��][��RGB LED��]
//uint8_t Arm_index = 0;//�糵�����(0~3)
uint8_t row_index[5] = {0};//5���۸��Ե���ָ��(0~4)
uint8_t RGB_index[5] = {0};//5���۸��Ե�RGBָ��(0~MAX_RGB_NUM-1)
uint8_t LED_index[5] = {0};//5���۸��Ե�LEDָ��(0~2)
uint8_t bit_index[5] = {1};//5���۸��Ե�����λָ��(0-7),��0���ź���SMD_LED_Color_Set�����ã��ж��дӵڶ����źſ�ʼ����

/** @brief  ������ˮ��Ч����
	* @param	[in]  arm	�Ʊ۱��
	* @param	[in]  mode ģʽ
  *					This parameter can be one of the following values:
	*         @arg ALL_ON: ȫ��(parameter������)
	*         @arg SLIDING_WINDOW: ��������(parameterΪ���ڴ�С)
	*         @arg TETRIS: ����˹����(parameter�����С)
	*         @arg CONVEYER_BELT: ���ʹ�(parameterΪ����/������)
	* @param	[in]  parameter ����
	* @param	[in]  color ��ʾ����ɫ
  *					This parameter can be one of the following values:
	*         @arg GREEN: ��ɫ��ɫ
	*         @arg RED: 	��ɫ��ɫ
	*         @arg BLUE:  ��ɫ��ɫ(��ɫ�����|���ɴ���)
	*         @arg RAND:	���ɫ
	*         @arg RUNNING_WATER:	��ˮ��Ч
	* @details	���ڵ��øú����Դﵽ��ˮЧ��
	* @retval None
	*/
void SMD_LED_Running_Water_Effect_Configuration(uint8_t arm, uint8_t mode, uint8_t parameter, uint8_t color)
{
	int16_t RGB_ON_OFF_Configuration_index[5] = {0};//���浱ǰ�����¸����۵���ʼRGB���
	uint8_t row = 0;//��ǰ�������
	memset(Arm_LED_Data[arm], 0x00, sizeof(Arm_LED_Data[arm]));//ȫ������
	//��ÿ��RGB������������Ϣ����Arm_LED_Data
	for(; row<5; row++)
		switch(mode)
		{
			case ALL_ON:
				memset(Arm_LED_Data[arm][row], 0xff, sizeof(Arm_LED_Data[arm][row]));//ȫ����һ
				break;
			case SLIDING_WINDOW:
//				if(RGB_ON_OFF_Configuration_index[arm] < 0)
//					memset(Arm_LED_Data[arm][row][0], 0xff, (parameter+RGB_ON_OFF_Configuration_index[arm])*3);
//	//				memset(Arm_LED_Data[arm][MAX_RGB_NUM+RGB_ON_OFF_Configuration_index[arm]], 0xff, -RGB_ON_OFF_Configuration_index[arm]*3);
//				else if(RGB_ON_OFF_Configuration_index[arm] > MAX_RGB_NUM-parameter)
//					memset(Arm_LED_Data[arm][row][RGB_ON_OFF_Configuration_index[arm]], 0xff, (MAX_RGB_NUM-RGB_ON_OFF_Configuration_index[arm])*3);
//				else
//					memset(Arm_LED_Data[arm][row][RGB_ON_OFF_Configuration_index[arm]], 0xff, parameter*3);
//				RGB_ON_OFF_Configuration_index[arm]++;//�ۼ�
//				if(RGB_ON_OFF_Configuration_index[arm] >= MAX_RGB_NUM)
//					RGB_ON_OFF_Configuration_index[arm] = -parameter;//�������
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
	//			RGB_ON_OFF_Configuration_index[arm]++;//�ۼ�
	//			if(RGB_ON_OFF_Configuration_index[arm] >= parameter)
	//				RGB_ON_OFF_Configuration_index[arm] = -parameter;//�������
				break;
		}
	//����ɫ��Ϣ�����Arm_LED_Data
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
	//���õ�һ�е�һ��RGB�е���ɫLED pwm����ռ�ձȣ�
	if(Arm_LED_Data[arm][0][LED_index[1]/3][LED_index[1]%3] == 0xff)
		ARM1_PULSE = LOGIC_ONE_PULSE;
	else ARM1_PULSE = LOGIC_ZERO_PULSE;
	//����pwm�ж�
	__HAL_TIM_ENABLE_IT(&htim2,TIM_IT_UPDATE);
}

void SMD_LED_TIM2_IT(void)
{
	if(Arm_LED_Data[1][row_index[1]][LED_index[1]/3][LED_index[1]%3] == 0xff)
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





