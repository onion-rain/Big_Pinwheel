/** 
* @brief    ң��������ģʽ�ܿ�
* @details  This is the detail description. 
* @author   Onion rain
* @date     date 2018.11.9
* @version  1.0
* @par Copyright (c):  Onion rain
* @par ��־
*/

#include "My_Remote.h"
#include "My_Car.hpp"
#include "Dbus_Uart.h"
#include "can.h"
#include "define_all.h"
#include "Global_Variable.h"
#include "My_SMDLED.h"
#include "string.h"
#include <stdlib.h>

#define STARTING	0//<����ʱ����
#define RUNNING		1//<����ʱ����
#define ENDING		2//<����ʱ����

static uint8_t remote_mode=0;
static uint8_t last_mode= 0xFF; //��һ��ң������ֵ�����ڶԱ��л�ģʽ

extern int8_t RGB_Start_index[5][5];//������My)SMDLED.c���л�ģʽʱ�����ֹ��ͬģʽ�����
extern uint8_t RGB_Tail_num[5][5];//������My)SMDLED.c���л�ģʽʱ�����ֹ��ͬģʽ�����

uint8_t return_data = 0;//debugר��

static void Rand_Purity_Color(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, rand()%6+1);
			break;
		case RUNNING:
			break;
		case ENDING:
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			break;
	}
}
/** 
    * @brief �������ģʽ
*/
static void run_mode(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, RAND);
			break;
		case RUNNING:
			break;
		case ENDING:
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			break;
	}
}
static void Sliding_Window(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%100 == 0)
			{
				return_data = SMD_LED_Running_Water_Effect_Configuration(1, SLIDING_WINDOW, 10, RAND);
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			break;
	}
}
static void Conveyer_Belt(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%200 == 0)
			{
				return_data = SMD_LED_Running_Water_Effect_Configuration(1, CONVEYER_BELT, 3, RAND);
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			break;
	}
}
static void Progress_Bar_0(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%100 == 0)
			{
				return_data = SMD_LED_Running_Water_Effect_Configuration(1, PROGRESS_BAR_0, 0, RAND);
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			break;
	}
}
static void Progress_Bar_1(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%100 == 0)
			{
				return_data = SMD_LED_Running_Water_Effect_Configuration(1, PROGRESS_BAR_1, 0, RAND);
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			break;
	}
}
static void Progress_Bar_2(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%60 == 0)
			{
				return_data = SMD_LED_Running_Water_Effect_Configuration(1, PROGRESS_BAR_2, 10, RAND);
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			memset(RGB_Tail_num, 0x00, sizeof(RGB_Tail_num));
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			break;
	}
}
static void Tetris(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%100 == 0)
			{
				return_data = SMD_LED_Running_Water_Effect_Configuration(1, TETRIS, 10, RAND);
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			break;
	}
}
/** 
    * @brief ��ȫģʽ
*/
static void safe_mode(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			break;
		case RUNNING:
			break;
		case ENDING:
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, 0);
			break;
	}
}
/** 
* @brief  ң����mode�ַ�
* @param [in]  mode ң����s1*10+s2
* @param [in]  type ���в���
* @par ��־
*
*/
static void Remote_Distribute(uint8_t mode, uint8_t type)
{
	switch(mode)
	{
		case 22:safe_mode(type);break;//��ȫģʽ
		case 32:run_mode(type);break;	//����ģʽ
		case 13:Tetris(type);break;	//����˹����
		case 33:Rand_Purity_Color(type);break;//�����ɫ
		case 12:Sliding_Window(type);break;//��������
		case 21:Conveyer_Belt(type);break;//���ʹ�
		case 11:Progress_Bar_0(type);break;//���������
		case 31:Progress_Bar_1(type);break;//ͬ�������
		case 23:Progress_Bar_2(type);break;//��ˮ������
		default:break;
	}
	manager::CANSend();
}

//��׼���������;��幦���޹�
void Remote_Handle(void)
{
	remote_mode=RC_Ctl.rc.s1*10+RC_Ctl.rc.s2;   //��ң������ȡ��ǰģʽ
	if(remote_mode!=last_mode)                   //�����ǰģʽ��֮ǰ��ģʽ��һ���Ļ� 
	{
		Remote_Distribute(last_mode,ENDING);       //�˳�֮ǰ��ģʽ
		Remote_Distribute(remote_mode,STARTING);   //���õ�ǰģʽ ��ʼ����
//		Remote_Distribute(remote_mode,RUNNING);    //��һ�ε�ǰģʽ��running
		last_mode = remote_mode;
	}
	Remote_Distribute(remote_mode,RUNNING);      //������ǰģʽ
}



