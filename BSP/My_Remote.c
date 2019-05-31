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
#include "My_Buff.h"

#define STARTING	0//<����ʱ����
#define RUNNING		1//<����ʱ����
#define ENDING		2//<����ʱ����

static uint8_t remote_mode=0;
static uint8_t last_mode= 0xFF; //��һ��ң������ֵ�����ڶԱ��л�ģʽ

extern int8_t RGB_Start_index[5][5];//������My)SMDLED.c���л�ģʽʱ�����ֹ��ͬģʽ�����
extern uint8_t RGB_Tail_num[5][5];//������My)SMDLED.c���л�ģʽʱ�����ֹ��ͬģʽ�����

static uint8_t return_data = 0;//debugר��

static void Rand_Purity_Color(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			return_data = ARM_Inside_ligthting_effect(0, ALL_ON, 0, rand()%6+1);
			return_data = ARM_Inside_ligthting_effect(1, ALL_ON, 0, rand()%6+1);
			return_data = ARM_Inside_ligthting_effect(2, ALL_ON, 0, rand()%6+1);
			SMD_LED_PWM_Init();
			break;
		case RUNNING:
			break;
		case ENDING:
			return_data = ARM_Inside_ligthting_effect(0, ALL_ON, 0, 0);
			return_data = ARM_Inside_ligthting_effect(1, ALL_ON, 0, 0);
			return_data = ARM_Inside_ligthting_effect(2, ALL_ON, 0, 0);
			SMD_LED_PWM_Init();
			break;
	}
}
static void Rand_Color(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			clear_with_purity_color(RAND);
			SMD_LED_PWM_Init();
			break;
		case RUNNING:
			break;
		case ENDING:
			clear_with_purity_color(0);
			SMD_LED_PWM_Init();
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
				return_data = ARM_Inside_ligthting_effect(0, SLIDING_WINDOW, 10, RED);
				return_data = ARM_Inside_ligthting_effect(1, SLIDING_WINDOW, 10, RED);
				return_data = ARM_Inside_ligthting_effect(2, SLIDING_WINDOW, 10, RED);
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			clear_with_purity_color(0);
			SMD_LED_PWM_Init();
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
				return_data = ARM_Inside_ligthting_effect(0, CONVEYER_BELT, 3, BLUE);
				return_data = ARM_Inside_ligthting_effect(1, CONVEYER_BELT, 3, BLUE);
				return_data = ARM_Inside_ligthting_effect(2, CONVEYER_BELT, 3, BLUE);
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			clear_with_purity_color(0);
			SMD_LED_PWM_Init();
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
				return_data = ARM_Inside_ligthting_effect(0, PROGRESS_BAR_0, 0, RED);
				return_data = ARM_Inside_ligthting_effect(1, PROGRESS_BAR_0, 0, RED);
				return_data = ARM_Inside_ligthting_effect(2, PROGRESS_BAR_0, 0, RED);
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			clear_with_purity_color(0);
			SMD_LED_PWM_Init();
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
				return_data = ARM_Inside_ligthting_effect(0, PROGRESS_BAR_1, 1, BLUE);
				return_data = ARM_Inside_ligthting_effect(1, PROGRESS_BAR_1, 1, BLUE);
				return_data = ARM_Inside_ligthting_effect(2, PROGRESS_BAR_1, 1, BLUE);
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			clear_with_purity_color(0);
			SMD_LED_PWM_Init();
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
			if(HAL_GetTick()%85 == 0)
			{
				return_data = ARM_Inside_ligthting_effect(0, PROGRESS_BAR_2, 3, BLUE);
				return_data = ARM_Inside_ligthting_effect(1, PROGRESS_BAR_2, 3, BLUE);
				return_data = ARM_Inside_ligthting_effect(2, PROGRESS_BAR_2, 3, BLUE);
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			memset(RGB_Tail_num, 0x00, sizeof(RGB_Tail_num));
			clear_with_purity_color(0);
			SMD_LED_PWM_Init();
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
				return_data = ARM_Inside_ligthting_effect(0, TETRIS, 10, RAND);
				return_data = ARM_Inside_ligthting_effect(1, TETRIS, 10, RAND);
				return_data = ARM_Inside_ligthting_effect(2, TETRIS, 10, RAND);
				SMD_LED_PWM_Init();
			}
			break;
		case ENDING:
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			clear_with_purity_color(0);
			SMD_LED_PWM_Init();
			break;
	}
}
#ifdef MASTER_CONTROL//����
static void run(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			hit[0] = 1;//������һ����
			break;
		case RUNNING:
			if(HAL_GetTick()-LastShootTick>2500 && last_arm_flash!=0x00)//���ʧ��
			{
				RC_Ctl.rc.s1 = 0;//��װң����������ȫ���������
				RC_Ctl.rc.s2 = 0;
				LastShootTick = HAL_GetTick();
			}else
				if(HAL_GetTick()%80 == 0)
					buff_flash();//���ˢ��
			break;
		case ENDING:
			buff_reset();
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			SMD_LED_PWM_Init();
			break;
	}
}
#endif
#ifdef SECONDARY_CONTROL
static void run(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%80 == 0)
				buff_flash();//���ˢ��
			break;
		case ENDING:
			buff_reset();
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			SMD_LED_PWM_Init();
			break;
	}
}
#endif
#ifdef THIRD_CONTROL
static void run(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%80 == 0)
				buff_flash();//���ˢ��
			break;
		case ENDING:
			buff_reset();
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			SMD_LED_PWM_Init();
			break;
	}
}
#endif
#ifdef FOURTH_CONTROL
static void run(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			break;
		case RUNNING:
			if(HAL_GetTick()%80 == 0)
				buff_flash();//���ˢ��
			break;
		case ENDING:
			buff_reset();
			memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			SMD_LED_PWM_Init();
			break;
	}
}
#endif
/** 
    * @brief ��ȫģʽ
*/
static void safe_mode(uint8_t type)
{
	switch(type)
	{
		case STARTING:
			clear_with_purity_color(0);
			SMD_LED_PWM_Init();
			break;
		case RUNNING:
			break;
		case ENDING:
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
		case 21:Rand_Color(type);break;	//���ɫ
		case 13:Tetris(type);break;	//����˹����
		case 33:Conveyer_Belt(type);break;//���ʹ�
		case 12:Sliding_Window(type);break;//��������
		case 32:Progress_Bar_0(type);break;//���������
		case 11:Rand_Purity_Color(type);break;//�����ɫ
		case 31:Progress_Bar_1(type);break;//ͬ�������
		case 23:Progress_Bar_2(type);break;//��ˮ������
		case 44:run(type);break;//�������ģʽ
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
		last_mode = remote_mode;
	}
	Remote_Distribute(remote_mode,RUNNING);      //������ǰģʽ
	
	#ifdef MASTER_CONTROL
		can_buffer[0] = RC_Ctl.rc.s1*10+RC_Ctl.rc.s2;
		can_buffer[1] = arm_flash<<8 | last_arm_flash;
		can_buffer[2] = arm_flashed;
		can_buffer[3] = arm_rectangle_on<<8 | arm_Utype_on;
		can_send_msg(&hcan1, 0x100, can_buffer);//�������Ƿ���Ϣ
	#endif
}



