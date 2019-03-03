/** 
* @brief    全车电机定义初始化
* @details  This is the detail description. 
* @author   Onion rain
* @date     2018.11.2
* @version  1.0
* @par Copyright (c):  Onion rain
* @par 日志
*/

#include "My_Car.hpp"
#include "can.h"
#include "tim.h"
#include "define_all.h"
#include "Global_Variable.h"
#include "usart.h"

//电机参数设置
Motor_t DJI_Motor_3508(8192, 19);

//大符
pid Pinwheel_Spe_Pid(15, 0.1, 1, 1000, 7000);//丝滑的2006
pid Pinwheel_Dir_Pid(0.3, 0.001, 0, 1000, 20000);
softmotor Pinwheel(1, 0x207, &DJI_Motor_3508, &Pinwheel_Spe_Pid, &Pinwheel_Dir_Pid);

/** 
* @brief  全车电机初始化
* @retval  None
* @par 日志
*/
void MOTOR_Init_All(void)
{
	manager::CANSelect(&hcan1,NULL);//选can
//	Pinwheel.Enable_Block(4000, 200, 2);//大符电机启用堵转检测
	ARM1_PULSE = LOGIC_ONE_PULSE;
	ARM2_PULSE = LOGIC_ZERO_PULSE;
	ARM3_PULSE = LOGIC_ONE_PULSE;
	ARM4_PULSE = LOGIC_ONE_PULSE;
	ARM5_PULSE = LOGIC_ONE_PULSE;
	HAL_TIM_PWM_Start(ARM1_TIM,ARM1_CHANNEL);
	HAL_TIM_PWM_Start(ARM2_TIM,ARM2_CHANNEL);
	HAL_TIM_PWM_Start(ARM3_TIM,ARM3_CHANNEL);
	HAL_TIM_PWM_Start(ARM4_TIM,ARM4_CHANNEL);
	HAL_TIM_PWM_Start(ARM5_TIM,ARM5_CHANNEL);//初始化为所有rgb全亮
}
