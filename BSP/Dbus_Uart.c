/** 
* @brief    DBUS�弶֧�ְ�
* @details  DMA����
* @author   Onion rain
* @date     date 2018.10.15
* @version  1.0
* @par Copyright (c):  Onion rain
* @par ��־
*/
#include "Dbus_Uart.h"
#include "usart.h"
#include "define_all.h"
#include "Global_Variable.h"
#include "My_Car.hpp"

static uint8_t DBUS_RX_Buffer[25];//ԭʼ����
RC_Ctl_t RC_Ctl;//������

void Dbus_Uart_Init(void)
{
	__HAL_UART_CLEAR_IDLEFLAG(&Dbus_Uart);//����жϱ�־λ
	__HAL_UART_ENABLE_IT(&Dbus_Uart,UART_IT_IDLE);//ʹ�ܴ��ڿ����ж�
	HAL_UART_Receive_DMA(&Dbus_Uart,(uint8_t*)DBUS_RX_Buffer,25);
}

static void Dbus_Uart_Calculate(void)
{
	RC_Ctl.rc.ch0 = ((int16_t)DBUS_RX_Buffer[0] | ((int16_t)DBUS_RX_Buffer[1] << 8)) & 0x07FF;
	RC_Ctl.rc.ch0-=1024;
  RC_Ctl.rc.ch1 = (((int16_t)DBUS_RX_Buffer[1] >> 3) | ((int16_t)DBUS_RX_Buffer[2] << 5)) & 0x07FF;
	RC_Ctl.rc.ch1-=1024;
  RC_Ctl.rc.ch2 = (((int16_t)DBUS_RX_Buffer[2] >> 6) | ((int16_t)DBUS_RX_Buffer[3] << 2) | ((int16_t)DBUS_RX_Buffer[4] << 10)) & 0x07FF;
	RC_Ctl.rc.ch2-=1024;
  RC_Ctl.rc.ch3 = (((int16_t)DBUS_RX_Buffer[4] >> 1) | ((int16_t)DBUS_RX_Buffer[5]<<7)) & 0x07FF;
  RC_Ctl.rc.ch3-=1024;
  RC_Ctl.rc.s1 = ((DBUS_RX_Buffer[5] >> 4) & 0x000C) >> 2;
  RC_Ctl.rc.s2 = ((DBUS_RX_Buffer[5] >> 4) & 0x0003);
 
	RC_Ctl.mouse.x = ((int16_t)DBUS_RX_Buffer[6]) | ((int16_t)DBUS_RX_Buffer[7] << 8);
  RC_Ctl.mouse.y = ((int16_t)DBUS_RX_Buffer[8]) | ((int16_t)DBUS_RX_Buffer[9] << 8);
  RC_Ctl.mouse.z = ((int16_t)DBUS_RX_Buffer[10]) | ((int16_t)DBUS_RX_Buffer[11] << 8);
  RC_Ctl.mouse.press_l = DBUS_RX_Buffer[12];
  RC_Ctl.mouse.press_r = DBUS_RX_Buffer[13];
  RC_Ctl.key.v = ((int16_t)DBUS_RX_Buffer[14]);
}

void Dbus_Uart_IT(void)
{
	if(__HAL_UART_GET_FLAG(&Dbus_Uart,UART_FLAG_IDLE)!=RESET)//�����ж�
	{		
		HAL_UART_DMAStop(&Dbus_Uart);  // ��dma
		Dbus_Uart_Calculate();// ��������
		__HAL_UART_CLEAR_IDLEFLAG(&Dbus_Uart);  //
		//HAL_UART_DMAResume(&Dbus_Uart);
		HAL_UART_Receive_DMA(&Dbus_Uart,(uint8_t*)DBUS_RX_Buffer,25);//��dma
	}
}


