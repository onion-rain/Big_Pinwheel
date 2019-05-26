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

#ifdef SECONDARY_CONTROL//����
	//�����������
	Motor_t DJI_Motor_3508(8192, 19);
	//���
	pid Pinwheel_Spe_Pid(15, 0.1, 1, 1000, 7000);
	pid Pinwheel_Dir_Pid(0.3, 0.001, 0, 1000, 20000);
	softmotor Pinwheel(1, 0x207, &DJI_Motor_3508, &Pinwheel_Spe_Pid, &Pinwheel_Dir_Pid);
#endif

/** 
* @brief  ȫ�������ʼ��
* @retval  None
* @par ��־
*/
void MOTOR_Init_All(void)
{
	#ifdef SECONDARY_CONTROL//����
		manager::CANSelect(&hcan1,NULL);//ѡcan
	#endif
	__HAL_TIM_ENABLE_IT(ARM_TIM,TIM_IT_UPDATE);
	HAL_TIM_PWM_Start(ARM_TIM,ARM0_CHANNEL);
	HAL_TIM_PWM_Start(ARM_TIM,ARM1_CHANNEL);
	HAL_TIM_PWM_Start(ARM_TIM,ARM2_CHANNEL);
}

