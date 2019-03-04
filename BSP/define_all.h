/** 
  * @brief   宏定义汇总
  * @details 
  * @author  Onion rain
  * @date    2019.3.3
  * @version  
  * @par     Copyright (c):  Onion rain
  * @par     V1.0
*/  

#ifndef __DEFINE_ALL_H
#define __DEFINE_ALL_H

//#include "FreeRTOSConfig.h"
/*
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
  * @brief pwm定时器通道定义
  * @refer My_Car.cpp
*/
#define ARM1_TIM &htim2
#define ARM1_CHANNEL TIM_CHANNEL_1
#define ARM1_PULSE TIM2->CCR1

#define ARM2_TIM &htim2
#define ARM2_CHANNEL TIM_CHANNEL_2
#define ARM2_PULSE TIM2->CCR2

#define ARM3_TIM &htim2
#define ARM3_CHANNEL TIM_CHANNEL_3
#define ARM3_PULSE TIM2->CCR3

#define ARM4_TIM &htim2
#define ARM4_CHANNEL TIM_CHANNEL_4
#define ARM4_PULSE TIM3->CCR4

#define ARM5_TIM &htim3
#define ARM5_CHANNEL TIM_CHANNEL_3
#define ARM5_PULSE TIM3->CCR3

#define LOGIC_ONE_PULSE (34-1)
#define LOGIC_ZERO_PULSE (13-1)
/** 
  * @brief 音乐播放
  * @refer music.h
*/
//#define DANCE
#define MTIM	htim8//定时器选择
#define MCH		TIM_CHANNEL_4
/** 
  * @brief DBUS 串口定义
  * @refer Dbus_Uart.h
	* @details 更改串口后中断处理函数也要改
*/
#define Dbus_Uart huart1
/** 
  * @brief RGB灯时钟通道定义
  * @refer My_RGB.h
*/
#define GREEN_ON HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET)
#define GREEN_OFF HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET)
#define RED_ON HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)
#define RED_OFF HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)
#define BLUE_ON HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
#define BLUE_OFF HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)

/*********************通用**************************/

/** 
  * @brief 绝对值函数
  * @refer Dbus_Uart.c
*/
#define ABS(x) ((x)>0?(x):-(x))

#endif
