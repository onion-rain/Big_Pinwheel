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

#define MAX_LED_NUM 288

uint8_t Arm_RGB_Data[5][MAX_LED_NUM][3] = {0xff};
int16_t LED_Configuration_index[5] = {0};//SMD_LED_Running_Water_Effect_Configuration�е���ʱ�����е���ʱ���������浱ǰ�����µ���ʼ�Ʊ��
//uint8_t Arm_index = 0;//����۴���(0~3)
uint16_t LED_index[5] = {0};//5���۸��Եĵƺ���(0~144*3)
uint8_t bit_index = 1;//ÿ����ɫ8λ(0~7)//��0���ź���SMD_LED_Color_Set�����ã��ж��дӵڶ����źſ�ʼ����

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
			LED_Configuration_index[arm]++;//�ۼ�
			if(LED_Configuration_index[arm] >= MAX_LED_NUM)
				LED_Configuration_index[arm] = -parameter;//�������
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
			LED_Configuration_index[arm]++;//�ۼ�
			if(LED_Configuration_index[arm] >= parameter)
				LED_Configuration_index[arm] = -parameter;//�������
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
	//���õ�һ��pwm����ռ�ձ�
	if(Arm_RGB_Data[1][LED_index[1]/3][LED_index[1]%3] == 0xff)
		ARM1_PULSE = LOGIC_ONE_PULSE;
	else ARM1_PULSE = LOGIC_ZERO_PULSE;
	//����pwm���
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
	if(bit_index == 8)//һ���Ƶ�8λ���ݱ�����
	{
		bit_index = 0;
		LED_index[1]++;
	}
	if(LED_index[1] == MAX_LED_NUM*3+1)//��һ�������8λ����
	{
//		Arm_index = 0;
		LED_index[1] = 0;
		bit_index = 1;
//		HAL_TIM_PWM_Stop(ARM1_TIM,ARM1_CHANNEL);
		__HAL_TIM_DISABLE_IT(&htim2,TIM_IT_UPDATE);
		ARM1_PULSE = 0;
	}
}





