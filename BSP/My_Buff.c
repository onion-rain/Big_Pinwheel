/** 
* @brief    ���
* @details  
* @author   Onion rain
* @date     2019.3.21
* @version  1.0
* @par Copyright (c):  Onion rain
* @par ��־
*/

#include "My_Buff.h"
#include "My_SMDLED.h"
#include "define_all.h"
#include "string.h"
#include <stdlib.h>

extern int8_t RGB_Start_index[5][5];//������My)SMDLED.c���л�ģʽʱ�����ֹ��ͬģʽ�����
extern uint8_t RGB_Tail_num[5][5];//������My)SMDLED.c���л�ģʽʱ�����ֹ��ͬģʽ�����

static uint8_t return_data = 0;//debugר��
uint8_t arm_flash = 0x00;//can_buffer[1]��8λ��ģʽ��ÿһλ����һ���ۣ�1����ǰ����Ҫˢ�£�0��ʾ������״��ȫ0��ʾȫ��
uint8_t last_arm_flash = 0x00;//�ϴα������ı�
uint8_t arm_flashed = 0x00;//��ˢ�¹��ı�
#ifdef AUXILIARY
	uint8_t flag_auxiliary = 0;//����ר�����״ν������ɹ���Чģʽ��־
#endif

void clear_with_purity_color(uint8_t color)
{
	SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, color);
	SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, color);
	SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, color);
}

void buff_conveyer_belt(void)
{
	#ifndef AUXILIARY//����
	if((arm_flash>>0)&0x01)//�ж��Ƿ�ˢ�´��ʹ�
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, CONVEYER_BELT, 3, BLUE);
		if((arm_flash>>1)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, CONVEYER_BELT, 3, BLUE);
		if((arm_flash>>2)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, CONVEYER_BELT, 3, BLUE);
	#else//����
		if((arm_flash>>3)&0x01)//�ж��Ƿ�ˢ�´��ʹ�
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, CONVEYER_BELT, 3, BLUE);
		if((arm_flash>>4)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, CONVEYER_BELT, 3, BLUE);
		if((arm_flash>>5)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, CONVEYER_BELT, 3, BLUE);
	#endif
}

void buff_all_on(void)
{
	#ifndef AUXILIARY//����
		if((last_arm_flash>>0)&0x01)//�ж��Ƿ�ˢ��ȫ��
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, BLUE);
		if((last_arm_flash>>1)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, BLUE);
		if((last_arm_flash>>2)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, BLUE);
	#else//����
		if((last_arm_flash>>3)&0x01)//�ж��Ƿ�ˢ��ȫ��
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, BLUE);
		if((last_arm_flash>>4)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, BLUE);
		if((last_arm_flash>>5)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, BLUE);
	#endif
}

uint8_t buff_sucess_process_var(void)
{
	SMD_LED_Running_Water_Effect_Configuration(0, PROGRESS_BAR_1, 4, BLUE);
	SMD_LED_Running_Water_Effect_Configuration(1, PROGRESS_BAR_1, 4, BLUE);
	return SMD_LED_Running_Water_Effect_Configuration(2, PROGRESS_BAR_1, 4, BLUE);
}

void buff_flag_reset(void)
{
	arm_flash = 0x00;
	arm_flashed = 0x00;
	last_arm_flash = 0x00;
	#ifdef AUXILIARY
		flag_auxiliary = 0;//����ר�����״ν������ɹ���Чģʽ��־
	#endif
}

void buff_flag_sucess(void)
{
	arm_flash = 0xff;
	arm_flashed = 0xff;
	last_arm_flash = 0xff;
}

void buff_flash(void)
{
	if(arm_flashed == 0xff && arm_flash == 0xff && last_arm_flash == 0xff)//����ɹ���Ч
	{
		#ifdef AUXILIARY//����
			if(flag_auxiliary == 0)
				memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			flag_auxiliary = 1;
		#endif
		if(buff_sucess_process_var() == 0)//��������ɣ������ʼ��
//			buff_flag_reset();
		;
	}
//	else if(arm_flash == 0x00 && arm_flashed == 0x00 && last_arm_flash == 0x00)//�ɹ������Ч��ɣ�����ָ��
//	{
//		memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
//	}
	else
	{
		buff_conveyer_belt();
		buff_all_on();
	}
	SMD_LED_PWM_Init();
}

void buff_new_armnum_produce(void)
{
	if(arm_flashed == 0x1f)/*��ȫ����ˢ�¹�*/
	{
		memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
		buff_flag_sucess();
	}else if(arm_flashed == 0xff && arm_flash == 0xff && last_arm_flash == 0xff)/*���ڵȴ��ɹ������Ч���*/
	{
		
	}else
	{
		last_arm_flash = arm_flash;//�����ϴθ��µı۱��
		do arm_flash = 0x01<<rand()%5;//���������һ��Ŀ��۱��
		while((arm_flash&arm_flashed) != 0x00);//Ŀ����Ѿ���ˢ�¹�����������
		arm_flashed |= arm_flash;//�����ѱ�ˢ�¹��ı�
	}
}
