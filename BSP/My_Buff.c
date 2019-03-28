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
#include "can.h"
#include "Can_Driver.hpp"
#include "Global_Variable.h"
#include <stdlib.h>

#define BUFF_COLOR RED

extern int8_t RGB_Start_index[5][5];//������My)SMDLED.c���л�ģʽʱ�����ֹ��ͬģʽ�����
extern uint8_t RGB_Tail_num[5][5];//������My)SMDLED.c���л�ģʽʱ�����ֹ��ͬģʽ�����

static uint8_t return_data = 0;//debugר��
uint8_t arm_flash = 0x00;//can_buffer[1]��8λ��ģʽ��ÿһλ����һ���ۣ�1����ǰ����Ҫˢ�£�0��ʾ������״��ȫ0��ʾȫ��
uint8_t last_arm_flash = 0x00;//�ϴα������ı�
uint8_t arm_flashed = 0x00;//��ˢ�¹��ı�
TickType_t LastShootTick;

#ifdef AUXILIARY//����
	uint8_t flag_auxiliary = 0;//����ר�����״ν������ɹ���Чģʽ��־
#endif

#ifndef AUXILIARY//����
	uint16_t Unprogrammable_Light_Bar = 0x0000;//���ɱ�̵ĵ���
	uint8_t hit[17] = {0};//��¼���װ�װ�Ĵ������
	uint8_t auxiliary_finished_flag = 0;//������ɴ���ɹ���Ч��־��1Ϊ����ɣ���can�ص������и���

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)//װ�װ��ܵ�����ص�����
{
	switch(GPIO_Pin)
	{
		case GPIO_PIN_6:
			if(HAL_GetTick()-LastShootTick > 100)
			{
				hit[0x10]++;
				LastShootTick = HAL_GetTick();
			}
			break;
		case GPIO_PIN_15:
			if(HAL_GetTick()-LastShootTick > 100)
			{
				hit[0x08]++;
				LastShootTick = HAL_GetTick();
			}
			break;
		case GPIO_PIN_14:
			if(HAL_GetTick()-LastShootTick > 100)
			{
				hit[0x04]++;
				LastShootTick = HAL_GetTick();
			}
			break;
		case GPIO_PIN_13:
			if(HAL_GetTick()-LastShootTick > 100)
			{
				hit[0x02]++;
				LastShootTick = HAL_GetTick();
			}
			break;
		case GPIO_PIN_12:
			if(HAL_GetTick()-LastShootTick > 100)
			{
				hit[0x01]++;
				LastShootTick = HAL_GetTick();
			}
			break;
	}
}
#endif

void clear_with_purity_color(uint8_t color)//�Դ�ɫ���
{
	SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, color);
	SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, color);
	SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, color);
}

void buff_conveyer_belt(void)//�����Ч
{
	#ifndef AUXILIARY//����
	if((arm_flash>>0)&0x01)//�ж��Ƿ�ˢ�´��ʹ�
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>1)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>2)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, CONVEYER_BELT, 3, BUFF_COLOR);
	#else//����
		if((arm_flash>>3)&0x01)//�ж��Ƿ�ˢ�´��ʹ�
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>4)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>5)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, CONVEYER_BELT, 3, BUFF_COLOR);
	#endif
}

void buff_all_on(void)//�ж���Щ������ɻ�������Ϊȫ��
{
	#ifndef AUXILIARY//����
		if((last_arm_flash>>0)&0x01)//�ж��Ƿ�ˢ��ȫ��
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>1)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>2)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, BUFF_COLOR);
	#else//����
		if((last_arm_flash>>3)&0x01)//�ж��Ƿ�ˢ��ȫ��
			return_data = SMD_LED_Running_Water_Effect_Configuration(0, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>4)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(1, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>5)&0x01)
			return_data = SMD_LED_Running_Water_Effect_Configuration(2, ALL_ON, 0, BUFF_COLOR);
	#endif
}

uint8_t buff_sucess_process_var(void)//���ȫ������ɹ���Ч
{
	SMD_LED_Running_Water_Effect_Configuration(0, PROGRESS_BAR_1, 4, BUFF_COLOR);
	SMD_LED_Running_Water_Effect_Configuration(1, PROGRESS_BAR_1, 4, BUFF_COLOR);
	return SMD_LED_Running_Water_Effect_Configuration(2, PROGRESS_BAR_1, 4, BUFF_COLOR);
}

void buff_reset(void)//�����ʼ��
{
	clear_with_purity_color(0);//�����������
	#ifndef AUXILIARY//����
		hit[0] = 1;//������һ����
		auxiliary_finished_flag = 0;//���سɹ������Ч��ɱ�־����
		Unprogrammable_Light_Bar = 0x0000;//���ɱ�̵�������
	#endif
	arm_flash = 0x00;
	arm_flashed = 0x00;
	last_arm_flash = 0x00;
}

void buff_flag_sucess(void)//����ɹ���־λ����
{
	arm_flash = 0xff;
	arm_flashed = 0xff;
	last_arm_flash = 0xff;
}
#ifndef AUXILIARY//����
void buff_new_armnum_produce(void)//������Ҫˢ�µı�
{
	if(arm_flashed == 0x1f)/*��ȫ����ˢ�¹�*/
	{
		memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));//��ʼindex���㣬׼���ɹ������Ч������ʹ�ñ�־flag_auxiliary�ж��Ƿ�Ϊ�״ν��гɹ������Ч�ж��Ƿ�Ҫ����index����
		buff_flag_sucess();
	}else//����δ��ˢ�µı�
	{
		last_arm_flash = arm_flash;//�����ϴθ��µı۱��
		do arm_flash = 0x01<<rand()%5;//���������һ��Ŀ��۱��
		while((arm_flash&arm_flashed) != 0x00);//Ŀ����Ѿ���ˢ�¹�����������
		arm_flashed |= arm_flash;//�����ѱ�ˢ�¹��ı�
		Unprogrammable_Light_Bar |= arm_flash<<5;//�˿�ˢ�µı�װ�װ������
	}
}
#endif
void buff_flash(void)//���ˢ�º������߳������ڵ���
{
	#ifndef AUXILIARY//����
		if(arm_flash!=0xff && hit[arm_flash])//��ȷװ�װ屻����
		{
			Unprogrammable_Light_Bar |= arm_flash;//�˿�arm_flash��δ��ˢ�£�������ı۱۵�����
			buff_new_armnum_produce();//������Ҫˢ�µı�
			memset(hit, 0, 17);//װ�װ������������
		}
	#endif
	if(arm_flashed == 0xff && arm_flash == 0xff && last_arm_flash == 0xff)//����ɹ�
	{
		#ifdef AUXILIARY//����
			if(flag_auxiliary == 0)//�ж��Ƿ�Ϊ�״ν��гɹ������Ч�ж��Ƿ�Ҫ����index����,׼���ɹ������Ч
				memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			flag_auxiliary = 1;//�´α㲻��Ҫ����start_index����
			if(buff_sucess_process_var()==0 && flag_auxiliary<=3)//���ؽ�������ɲ����״ν��г�ʼ��(����ǰ���γ�ʼ�������ط��ͽ�������ɵ���Ϣ��֮���ٷ���)
			{
				buff_reset();
				flag_auxiliary++;//����ר�����״ν������ɹ���Чģʽ��־
				can_buffer[0] = 1;
				can_send_msg(&hcan1, 0x222, can_buffer);//�����ط���Ϣ
			}
		#else //����
		if(buff_sucess_process_var() == 0)//���ؽ�������ɣ��ȴ��������
			if(auxiliary_finished_flag == 1)//���ؽ�������ɣ������ʼ��
				buff_reset();
			#endif
	}else
	{
		#ifdef AUXILIARY//����
			flag_auxiliary = 0;
		#endif
		buff_conveyer_belt();//���ݱ�־λѡ��۽��д��ʹ���Ч��ʾ
		buff_all_on();//���ݱ�־λѡ��۽���ȫ����ʾ
	}
	SMD_LED_PWM_Init();
}

