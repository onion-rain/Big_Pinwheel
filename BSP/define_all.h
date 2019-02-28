/** 
  * @brief   宏定义汇总
  * @details 
  * @author  Onion rain
  * @date    2018.11.15
  * @version  
  * @par     Copyright (c):  Onion rain
  * @par     V1.0
  */  

#ifndef __DEFINE_ALL_H
#define __DEFINE_ALL_H

//#include "FreeRTOSConfig.h"
/*
	* 云台电机yaw：can1 0x205
	* 云台电机pitch：can1 0x206
	* 拨弹电机：can1 0x207
	* 摩擦轮电机(3510)：can2 0x201;0x202
	* Middlewares/FreeRTOS group改为--c99
	* 注释掉freertos.c中的defaultTask
	* freertos生成后配置FreeRTOSConfig.h 122行：#define INCLUDE_vTaskDelayUntil 1
	* dma.c中dma接收中断注释掉 DMA2_Stream1(usart6_rx_dma)
	*
	*若要在MonitorTask中打印任务执行情况：
		*在main.c中USER CODE BEGIN 0添加volatile uint32_t ulHighFrequencyTimerTicks = 0;
		*在HAL_TIM_PeriodElapsedCallback添加
				if(htim->Instance == htim2.Instance)
					ulHighFrequencyTimerTicks++;//自定义计数器
		*在FreeRTOSConfig.h中USER CODE BEGIN Includes添加
				extern volatile uint32_t ulHighFrequencyTimerTicks;
				#define configUSE_TRACE_FACILITY                     1
				#define configGENERATE_RUN_TIME_STATS                1
				#define configUSE_STATS_FORMATTING_FUNCTIONS         1
				#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()     (ulHighFrequencyTimerTicks = 0ul) 
				#define portGET_RUN_TIME_COUNTER_VALUE()             ulHighFrequencyTimerTicks
*/

/**********************便于移植**********************/
/** 
  * @brief 摩擦轮时钟通道定义
  * @refer My_Car.cpp
*/
#define FRICTION_L_TIM &htim3
#define FRICTION_L_CHANNEL TIM_CHANNEL_3
#define FRICTION_L_SPEED_SET TIM3->CCR3
#define FRICTION_R_TIM &htim3
#define FRICTION_R_CHANNEL TIM_CHANNEL_4
#define FRICTION_R_SPEED_SET TIM3->CCR4
#define FRICTION_PWM_MAX 2000    //摩擦轮PWM最大值
#define FRICTION_PWM_MIN 1000    //摩擦轮PWM最小值
#define FRICTION_PWM_SAFE 1000   //摩擦轮安全模式PWM输出值
#define FRICTION_SPE_MAX 4000    //摩擦轮速度最大值(rpm)
#define FRICTION_SPE_MIN 0       //摩擦轮速度最小值(rpm)
/** 
  * @brief DBUS 串口定义
  * @refer Dbus_Uart.h
	* @details 更改串口后中断处理函数也要改
*/
#define Dbus_Uart huart1
/** 
  * @brief 裁判系统串口定义
  * @refer Judgment_Uart.h
	* @details 更改串口后中断处理函数也要改
*/
/*********************通用**************************/

/** 
  * @brief 绝对值函数
  * @refer Dbus_Uart.c
*/
#define ABS(x) ((x)>0?(x):-(x))

#endif
