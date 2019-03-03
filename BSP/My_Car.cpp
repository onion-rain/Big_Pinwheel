/** 
* @brief    ȫ����������ʼ��
* @details  This is the detail description. 
* @author   Onion rain
* @date     2018.11.2
* @version  1.0
* @par Copyright (c):  Onion rain
* @par ��־
*/

#include "My_Car.hpp"
#include "can.h"
#include "tim.h"
#include "define_all.h"
#include "Global_Variable.h"
#include "usart.h"

//�����������
Motor_t DJI_Motor_3508(8192, 19);

//���
pid Pinwheel_Spe_Pid(15, 0.1, 1, 1000, 7000);//˿����2006
pid Pinwheel_Dir_Pid(0.3, 0.001, 0, 1000, 20000);
softmotor Pinwheel(1, 0x207, &DJI_Motor_3508, &Pinwheel_Spe_Pid, &Pinwheel_Dir_Pid);

/** 
* @brief  ȫ�������ʼ��
* @retval  None
* @par ��־
*/
void MOTOR_Init_All(void)
{
	manager::CANSelect(&hcan1,NULL);//ѡcan
//	Pinwheel.Enable_Block(4000, 200, 2);//���������ö�ת���
	ARM1_PULSE = LOGIC_ONE_PULSE;
	ARM2_PULSE = LOGIC_ZERO_PULSE;
	ARM3_PULSE = LOGIC_ONE_PULSE;
	ARM4_PULSE = LOGIC_ONE_PULSE;
	ARM5_PULSE = LOGIC_ONE_PULSE;
	HAL_TIM_PWM_Start(ARM1_TIM,ARM1_CHANNEL);
	HAL_TIM_PWM_Start(ARM2_TIM,ARM2_CHANNEL);
	HAL_TIM_PWM_Start(ARM3_TIM,ARM3_CHANNEL);
	HAL_TIM_PWM_Start(ARM4_TIM,ARM4_CHANNEL);
	HAL_TIM_PWM_Start(ARM5_TIM,ARM5_CHANNEL);//��ʼ��Ϊ����rgbȫ��
}
