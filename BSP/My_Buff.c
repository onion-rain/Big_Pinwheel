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

#define BUFF_COLOR ORANGE_x//RED//CYAN_x//

extern int8_t RGB_Start_index[5][5];//������My_SMDLED.c���л�ģʽʱ�����ֹ��ͬģʽ�����
extern uint8_t RGB_Tail_num[5][5];//������My_SMDLED.c���л�ģʽʱ�����ֹ��ͬģʽ�����

static uint8_t return_data = 0;//debugר��

uint8_t arm_flash = 0x00;//[0-4]��ÿһλ����һ�����м����1����ǰ����Ҫˢ��(��ˮ��Ч)��0��ʾ������״
uint8_t last_arm_flash = 0x00;//[0-4]��ÿһλ����һ�����м����1����ǰ������Ҫȫ����0��ʾ������״
uint8_t arm_flashed = 0x00;//��ˢ�¹��ı�

uint8_t arm_rectangle_on = 0x00;//[0-4]��ÿһλ����һ�����ⲿ����1����ǰ���������Ҫȫ����0��ʾ������״
uint8_t arm_Utype_on = 0x00;//[6-10]��ÿһλ����һ�����ⲿU�͵���1����ǰU�͵�����Ҫȫ����0��ʾ������״

TickType_t LastShootTick[17];

#ifdef SECONDARY_CONTROL//����
	uint8_t flag_secondary = 0;//����ר�����״ν������ɹ���Чģʽ��־
#endif

#ifdef MASTER_CONTROL//����
	uint8_t hit[17] = {0};//��¼���װ�װ�Ĵ������
	uint8_t secondary_finished_flag = 0;//������ɴ���ɹ���Ч��־��1Ϊ����ɣ���can�ص������и���
	int16_t exti_time = 0;//ʣ������װ�װ�����ʱ�䣬��ֹ��

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)//װ�װ��ܵ�����ص�����
{
	if(exti_time == 0)
	{
		exti_time = 3;
		switch(GPIO_Pin)
		{
			case GPIO_PIN_6:
				if(HAL_GetTick()-LastShootTick[0x10] > 100)
				{
					hit[0x10]++;
					LastShootTick[0x10] = HAL_GetTick();
				}
				break;
			case GPIO_PIN_15:
				if(HAL_GetTick()-LastShootTick[0x08] > 100)
				{
					hit[0x08]++;
					LastShootTick[0x08] = HAL_GetTick();
				}
				break;
			case GPIO_PIN_14:
				if(HAL_GetTick()-LastShootTick[0x04] > 100)
				{
					hit[0x04]++;
					LastShootTick[0x04] = HAL_GetTick();
				}
				break;
			case GPIO_PIN_13:
				if(HAL_GetTick()-LastShootTick[0x02] > 100)
				{
					hit[0x02]++;
					LastShootTick[0x02] = HAL_GetTick();
				}
				break;
			case GPIO_PIN_12:
				if(HAL_GetTick()-LastShootTick[0x01] > 100)
				{
					hit[0x01]++;
					LastShootTick[0x01] = HAL_GetTick();
				}
				break;
		}
	}
}
#endif

void clear_with_purity_color(uint8_t color)//�Դ�ɫ���
{
	ARM_Inside_ligthting_effect(0, ALL_ON, 0, color);
	ARM_Inside_ligthting_effect(1, ALL_ON, 0, color);
	ARM_Inside_ligthting_effect(2, ALL_ON, 0, color);
}

void buff_conveyer_belt(void)//�����Ч
{
	#ifdef MASTER_CONTROL//����
	if((arm_flash>>0)&0x01)//�ж��Ƿ�ˢ��(��ˮ��Ч)
			return_data = ARM_Inside_ligthting_effect(0, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>1)&0x01)
			return_data = ARM_Inside_ligthting_effect(1, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>2)&0x01)
			return_data = ARM_Inside_ligthting_effect(2, CONVEYER_BELT, 3, BUFF_COLOR);
	#endif
	#ifdef SECONDARY_CONTROL//����
		if((arm_flash>>3)&0x01)//�ж��Ƿ�ˢ��(��ˮ��Ч)
			return_data = ARM_Inside_ligthting_effect(0, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>4)&0x01)
			return_data = ARM_Inside_ligthting_effect(1, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>5)&0x01)
			return_data = ARM_Inside_ligthting_effect(2, CONVEYER_BELT, 3, BUFF_COLOR);
	#endif
	#ifdef THIRD_CONTROL//����
		if((arm_rectangle_on>>0)&0x01 && (arm_Utype_on>>0)&0x01)//ȫ��
			ARM_Outside_ligthting_effect(0, HITTED, BUFF_COLOR);
		else if((arm_rectangle_on>>0)&0x01 && !((arm_Utype_on>>0)&0x01))//���ε�����
			ARM_Outside_ligthting_effect(0, WAIT_HIT, BUFF_COLOR);
		else if(!((arm_rectangle_on>>0)&0x01) && !((arm_Utype_on>>0)&0x01))//ȫ��
			ARM_Outside_ligthting_effect(0, UNSETLECTED, BUFF_COLOR);
		else if(!((arm_rectangle_on>>0)&0x01) && ((arm_Utype_on>>0)&0x01))//U�͵���������Ӧ���ڸ��������debug��
			return_data = ARM_Inside_ligthting_effect(0, CONVEYER_BELT, 3, BUFF_COLOR);
		
		if((arm_rectangle_on>>1)&0x01 && (arm_Utype_on>>1)&0x01)//ȫ��
			ARM_Outside_ligthting_effect(1, HITTED, BUFF_COLOR);
		else if((arm_rectangle_on>>1)&0x01 && !((arm_Utype_on>>1)&0x01))//���ε�����
			ARM_Outside_ligthting_effect(1, WAIT_HIT, BUFF_COLOR);
		else if(!((arm_rectangle_on>>1)&0x01) && !((arm_Utype_on>>1)&0x01))//ȫ��
			ARM_Outside_ligthting_effect(1, UNSETLECTED, BUFF_COLOR);
		else if(!((arm_rectangle_on>>1)&0x01) && ((arm_Utype_on>>1)&0x01))//U�͵���������Ӧ���ڸ��������debug��
			return_data = ARM_Inside_ligthting_effect(1, CONVEYER_BELT, 3, BUFF_COLOR);
		
		if((arm_rectangle_on>>2)&0x01 && (arm_Utype_on>>2)&0x01)//ȫ��
			ARM_Outside_ligthting_effect(2, HITTED, BUFF_COLOR);
		else if((arm_rectangle_on>>2)&0x01 && !((arm_Utype_on>>2)&0x01))//���ε�����
			ARM_Outside_ligthting_effect(2, WAIT_HIT, BUFF_COLOR);
		else if(!((arm_rectangle_on>>2)&0x01) && !((arm_Utype_on>>2)&0x01))//ȫ��
			ARM_Outside_ligthting_effect(2, UNSETLECTED, BUFF_COLOR);
		else if(!((arm_rectangle_on>>2)&0x01) && ((arm_Utype_on>>2)&0x01))//U�͵���������Ӧ���ڸ��������debug��
			return_data = ARM_Inside_ligthting_effect(2, CONVEYER_BELT, 3, BUFF_COLOR);
	#endif
	#ifdef FOURTH_CONTROL//����
		if((arm_rectangle_on>>3)&0x01 && (arm_Utype_on>>3)&0x01)//ȫ��
			ARM_Outside_ligthting_effect(0, HITTED, BUFF_COLOR);
		else if((arm_rectangle_on>>3)&0x01 && !((arm_Utype_on>>3)&0x01))//���ε�����
			ARM_Outside_ligthting_effect(0, WAIT_HIT, BUFF_COLOR);
		else if(!((arm_rectangle_on>>3)&0x01) && !((arm_Utype_on>>3)&0x01))//ȫ��
			ARM_Outside_ligthting_effect(0, UNSETLECTED, BUFF_COLOR);
		else if(!((arm_rectangle_on>>3)&0x01) && ((arm_Utype_on>>3)&0x01))//U�͵���������Ӧ���ڸ��������debug��
			return_data = ARM_Inside_ligthting_effect(0, CONVEYER_BELT, 3, BUFF_COLOR);
			
		if((arm_rectangle_on>>4)&0x01 && (arm_Utype_on>>4)&0x01)//ȫ��
			ARM_Outside_ligthting_effect(1, HITTED, BUFF_COLOR);
		else if((arm_rectangle_on>>4)&0x01 && !((arm_Utype_on>>4)&0x01))//���ε�����
			ARM_Outside_ligthting_effect(1, WAIT_HIT, BUFF_COLOR);
		else if(!((arm_rectangle_on>>4)&0x01) && !((arm_Utype_on>>4)&0x01))//ȫ��
			ARM_Outside_ligthting_effect(1, UNSETLECTED, BUFF_COLOR);
		else if(!((arm_rectangle_on>>4)&0x01) && ((arm_Utype_on>>4)&0x01))//U�͵���������Ӧ���ڸ��������debug��
			ARM_Inside_ligthting_effect(1, CONVEYER_BELT, 3, BUFF_COLOR);
			
		if((arm_rectangle_on>>5)&0x01 && (arm_Utype_on>>5)&0x01)//ȫ��
			ARM_Outside_ligthting_effect(2, HITTED, BUFF_COLOR);
		else if((arm_rectangle_on>>5)&0x01 && !((arm_Utype_on>>5)&0x01))//���ε�����
			ARM_Outside_ligthting_effect(2, WAIT_HIT, BUFF_COLOR);
		else if(!((arm_rectangle_on>>5)&0x01) && !((arm_Utype_on>>5)&0x01))//ȫ��
			ARM_Outside_ligthting_effect(2, UNSETLECTED, BUFF_COLOR);
		else if(!((arm_rectangle_on>>5)&0x01) && ((arm_Utype_on>>5)&0x01))//U�͵���������Ӧ���ڸ��������debug��
			return_data = ARM_Inside_ligthting_effect(2, CONVEYER_BELT, 3, BUFF_COLOR);
	#endif
}

void buff_all_on(void)//�ж���Щ������ɻ�������Ϊȫ��
{
	#ifdef MASTER_CONTROL//����
		if((last_arm_flash>>0)&0x01)//�ж��Ƿ�ˢ��ȫ��
			return_data = ARM_Inside_ligthting_effect(0, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>1)&0x01)
			return_data = ARM_Inside_ligthting_effect(1, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>2)&0x01)
			return_data = ARM_Inside_ligthting_effect(2, ALL_ON, 0, BUFF_COLOR);
	#endif
	#ifdef SECONDARY_CONTROL//����
		if((last_arm_flash>>3)&0x01)//�ж��Ƿ�ˢ��ȫ��
			return_data = ARM_Inside_ligthting_effect(0, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>4)&0x01)
			return_data = ARM_Inside_ligthting_effect(1, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>5)&0x01)
			return_data = ARM_Inside_ligthting_effect(2, ALL_ON, 0, BUFF_COLOR);
	#endif
}

uint8_t buff_sucess_process_var(void)//���ȫ������ɹ���Ч
{
	#ifdef MASTER_CONTROL//����
		ARM_Inside_ligthting_effect(0, PROGRESS_BAR_1, 4, BUFF_COLOR);
		ARM_Inside_ligthting_effect(1, PROGRESS_BAR_1, 4, BUFF_COLOR);
		return ARM_Inside_ligthting_effect(2, PROGRESS_BAR_1, 4, BUFF_COLOR);
	#endif
	#ifdef SECONDARY_CONTROL//����
		ARM_Inside_ligthting_effect(0, PROGRESS_BAR_1, 4, BUFF_COLOR);
		ARM_Inside_ligthting_effect(1, PROGRESS_BAR_1, 4, BUFF_COLOR);
		return ARM_Inside_ligthting_effect(2, PROGRESS_BAR_1, 4, BUFF_COLOR);
	#endif
	#ifdef THIRD_CONTROL//����
		ARM_Outside_ligthting_effect(0, SUCCESS, BUFF_COLOR);
		ARM_Outside_ligthting_effect(1, SUCCESS, BUFF_COLOR);
		ARM_Outside_ligthting_effect(2, SUCCESS, BUFF_COLOR);
	#endif
	#ifdef FOURTH_CONTROL//����
		ARM_Outside_ligthting_effect(0, SUCCESS, BUFF_COLOR);
		ARM_Outside_ligthting_effect(1, SUCCESS, BUFF_COLOR);
		ARM_Outside_ligthting_effect(2, SUCCESS, BUFF_COLOR);
	#endif
	
}

void buff_reset(void)//�����ʼ��
{
	clear_with_purity_color(0);//�����������
	#ifdef MASTER_CONTROL//����
		hit[0] = 1;//������һ����
		secondary_finished_flag = 0;//���سɹ������Ч��ɱ�־����
	#endif
	arm_flash = 0;
	arm_flashed = 0;
	last_arm_flash = 0;
	arm_rectangle_on = 0;
	arm_Utype_on = 0;
}

void buff_flag_sucess(void)//����ɹ���־λ����
{
	arm_flash = 0xff;
	arm_flashed = 0xff;
	last_arm_flash = 0xff;
}
#ifdef MASTER_CONTROL//����
void buff_new_armnum_produce(void)//������Ҫˢ�µı�
{
	if(arm_flashed == 0x1f)/*��ȫ����ˢ�¹�*/
	{
		memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));//��ʼindex���㣬׼���ɹ������Ч������ʹ�ñ�־flag_secondary�ж��Ƿ�Ϊ�״ν��гɹ������Ч�ж��Ƿ�Ҫ����index����
		buff_flag_sucess();
	}else//����δ��ˢ�µı�
	{
		last_arm_flash = arm_flash;//�����ϴθ��µı۱��
		do arm_flash = 0x01<<rand()%5;//���������һ��Ŀ��۱��
		while((arm_flash&arm_flashed) != 0x00);//Ŀ����Ѿ���ˢ�¹�����������
		arm_flashed |= arm_flash;//�����ѱ�ˢ�¹��ı�
		arm_rectangle_on |= arm_flash;//�˿�ˢ�µı�װ�װ������
	}
}
#endif
void buff_flash(void)//���ˢ�º������߳������ڵ���
{
#ifdef MASTER_CONTROL//����
	exti_time--;//װ�װ��⵹��ʱ
	if(exti_time<0)exti_time = 0;
	if(arm_flash!=0xff && hit[arm_flash])//��ȷװ�װ屻����
	{
		arm_Utype_on |= arm_flash;//�˿�arm_flash��δ��ˢ�£�������ı۱۵�����
		buff_new_armnum_produce();//������Ҫˢ�µı�
		memset(hit, 0, 17);//װ�װ������������
	}
#endif
	if(arm_flashed == 0xff && arm_flash == 0xff && last_arm_flash == 0xff)//����ɹ�
	{
		#ifdef MASTER_CONTROL
		if(buff_sucess_process_var() == 0)//���ؽ�������ɣ��ȴ��������
			if(secondary_finished_flag == 1)//���ؽ�������ɣ������ʼ��
				buff_reset();
		#endif
		#ifdef SECONDARY_CONTROL//����
			if(flag_secondary == 0)//�ж��Ƿ�Ϊ�״ν��гɹ������Ч�ж��Ƿ�Ҫ����index����,׼���ɹ������Ч
				memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			flag_secondary = 1;//�´α㲻��Ҫ����start_index����
			if(buff_sucess_process_var()==0 && flag_secondary<=3)//���ؽ�������ɲ����״ν��г�ʼ��(����ǰ���γ�ʼ�������ط��ͽ�������ɵ���Ϣ��֮���ٷ���)
			{
				buff_reset();
				flag_secondary++;//����ר�����״ν������ɹ���Чģʽ��־
				can_buffer[0] = 1;
				can_send_msg(&hcan1, 0x201, can_buffer);//�����ط���Ϣ
			}
		#endif
		#ifdef THIRD_CONTROL//����
			buff_sucess_process_var();
		#endif
		#ifdef FOURTH_CONTROL//����
			buff_sucess_process_var();
		#endif
	}else
	{
		#ifdef SECONDARY_CONTROL//����
			flag_secondary = 0;
		#endif
		buff_conveyer_belt();//���ݱ�־λѡ��۽��д��ʹ���Ч��ʾ
		buff_all_on();//���ݱ�־λѡ��۽���ȫ����ʾ
	}
	SMD_LED_PWM_Init();
}

