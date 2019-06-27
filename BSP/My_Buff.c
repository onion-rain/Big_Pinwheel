/** 
* @brief    大符
* @details  
* @author   Onion rain
* @date     2019.3.21
* @version  1.0
* @par Copyright (c):  Onion rain
* @par 日志
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

extern int8_t RGB_Start_index[5][5];//声明于My_SMDLED.c，切换模式时清零防止不同模式间干扰
extern uint8_t RGB_Tail_num[5][5];//声明于My_SMDLED.c，切换模式时清零防止不同模式间干扰

static uint8_t return_data = 0;//debug专属

uint8_t arm_flash = 0x00;//[0-4]，每一位代表一个臂中间灯阵，1代表当前臂需要刷新(流水灯效)，0表示保持现状
uint8_t last_arm_flash = 0x00;//[0-4]，每一位代表一个臂中间灯阵，1代表当前灯阵需要全亮，0表示保持现状
uint8_t arm_flashed = 0x00;//已刷新过的臂

uint8_t arm_rectangle_on = 0x00;//[0-4]，每一位代表一个臂外部灯阵，1代表当前矩阵灯阵需要全亮，0表示保持现状
uint8_t arm_Utype_on = 0x00;//[6-10]，每一位代表一个臂外部U型灯阵，1代表当前U型灯阵需要全亮，0表示保持现状

TickType_t LastShootTick[17];

#ifdef SECONDARY_CONTROL//副控
	uint8_t flag_secondary = 0;//副控专属，首次进入打符成功灯效模式标志
#endif

#ifdef MASTER_CONTROL//主控
	uint8_t hit[17] = {0};//记录五个装甲板的打击次数
	uint8_t secondary_finished_flag = 0;//副控完成打符成功灯效标志，1为已完成，在can回调函数中更新
	int16_t exti_time = 0;//剩余允许装甲板检测打击时间，防止误触

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)//装甲板受到打击回调函数
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

void clear_with_purity_color(uint8_t color)//以纯色填充
{
	ARM_Inside_ligthting_effect(0, ALL_ON, 0, color);
	ARM_Inside_ligthting_effect(1, ALL_ON, 0, color);
	ARM_Inside_ligthting_effect(2, ALL_ON, 0, color);
}

void buff_conveyer_belt(void)//大符灯效
{
	#ifdef MASTER_CONTROL//主控
	if((arm_flash>>0)&0x01)//判断是否刷新(流水灯效)
			return_data = ARM_Inside_ligthting_effect(0, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>1)&0x01)
			return_data = ARM_Inside_ligthting_effect(1, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>2)&0x01)
			return_data = ARM_Inside_ligthting_effect(2, CONVEYER_BELT, 3, BUFF_COLOR);
	#endif
	#ifdef SECONDARY_CONTROL//副控
		if((arm_flash>>3)&0x01)//判断是否刷新(流水灯效)
			return_data = ARM_Inside_ligthting_effect(0, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>4)&0x01)
			return_data = ARM_Inside_ligthting_effect(1, CONVEYER_BELT, 3, BUFF_COLOR);
		if((arm_flash>>5)&0x01)
			return_data = ARM_Inside_ligthting_effect(2, CONVEYER_BELT, 3, BUFF_COLOR);
	#endif
	#ifdef THIRD_CONTROL//副控
		if((arm_rectangle_on>>0)&0x01 && (arm_Utype_on>>0)&0x01)//全亮
			ARM_Outside_ligthting_effect(0, HITTED, BUFF_COLOR);
		else if((arm_rectangle_on>>0)&0x01 && !((arm_Utype_on>>0)&0x01))//矩形灯阵亮
			ARM_Outside_ligthting_effect(0, WAIT_HIT, BUFF_COLOR);
		else if(!((arm_rectangle_on>>0)&0x01) && !((arm_Utype_on>>0)&0x01))//全灭
			ARM_Outside_ligthting_effect(0, UNSETLECTED, BUFF_COLOR);
		else if(!((arm_rectangle_on>>0)&0x01) && ((arm_Utype_on>>0)&0x01))//U型灯阵亮，不应存在该情况，仅debug用
			return_data = ARM_Inside_ligthting_effect(0, CONVEYER_BELT, 3, BUFF_COLOR);
		
		if((arm_rectangle_on>>1)&0x01 && (arm_Utype_on>>1)&0x01)//全亮
			ARM_Outside_ligthting_effect(1, HITTED, BUFF_COLOR);
		else if((arm_rectangle_on>>1)&0x01 && !((arm_Utype_on>>1)&0x01))//矩形灯阵亮
			ARM_Outside_ligthting_effect(1, WAIT_HIT, BUFF_COLOR);
		else if(!((arm_rectangle_on>>1)&0x01) && !((arm_Utype_on>>1)&0x01))//全灭
			ARM_Outside_ligthting_effect(1, UNSETLECTED, BUFF_COLOR);
		else if(!((arm_rectangle_on>>1)&0x01) && ((arm_Utype_on>>1)&0x01))//U型灯阵亮，不应存在该情况，仅debug用
			return_data = ARM_Inside_ligthting_effect(1, CONVEYER_BELT, 3, BUFF_COLOR);
		
		if((arm_rectangle_on>>2)&0x01 && (arm_Utype_on>>2)&0x01)//全亮
			ARM_Outside_ligthting_effect(2, HITTED, BUFF_COLOR);
		else if((arm_rectangle_on>>2)&0x01 && !((arm_Utype_on>>2)&0x01))//矩形灯阵亮
			ARM_Outside_ligthting_effect(2, WAIT_HIT, BUFF_COLOR);
		else if(!((arm_rectangle_on>>2)&0x01) && !((arm_Utype_on>>2)&0x01))//全灭
			ARM_Outside_ligthting_effect(2, UNSETLECTED, BUFF_COLOR);
		else if(!((arm_rectangle_on>>2)&0x01) && ((arm_Utype_on>>2)&0x01))//U型灯阵亮，不应存在该情况，仅debug用
			return_data = ARM_Inside_ligthting_effect(2, CONVEYER_BELT, 3, BUFF_COLOR);
	#endif
	#ifdef FOURTH_CONTROL//副控
		if((arm_rectangle_on>>3)&0x01 && (arm_Utype_on>>3)&0x01)//全亮
			ARM_Outside_ligthting_effect(0, HITTED, BUFF_COLOR);
		else if((arm_rectangle_on>>3)&0x01 && !((arm_Utype_on>>3)&0x01))//矩形灯阵亮
			ARM_Outside_ligthting_effect(0, WAIT_HIT, BUFF_COLOR);
		else if(!((arm_rectangle_on>>3)&0x01) && !((arm_Utype_on>>3)&0x01))//全灭
			ARM_Outside_ligthting_effect(0, UNSETLECTED, BUFF_COLOR);
		else if(!((arm_rectangle_on>>3)&0x01) && ((arm_Utype_on>>3)&0x01))//U型灯阵亮，不应存在该情况，仅debug用
			return_data = ARM_Inside_ligthting_effect(0, CONVEYER_BELT, 3, BUFF_COLOR);
			
		if((arm_rectangle_on>>4)&0x01 && (arm_Utype_on>>4)&0x01)//全亮
			ARM_Outside_ligthting_effect(1, HITTED, BUFF_COLOR);
		else if((arm_rectangle_on>>4)&0x01 && !((arm_Utype_on>>4)&0x01))//矩形灯阵亮
			ARM_Outside_ligthting_effect(1, WAIT_HIT, BUFF_COLOR);
		else if(!((arm_rectangle_on>>4)&0x01) && !((arm_Utype_on>>4)&0x01))//全灭
			ARM_Outside_ligthting_effect(1, UNSETLECTED, BUFF_COLOR);
		else if(!((arm_rectangle_on>>4)&0x01) && ((arm_Utype_on>>4)&0x01))//U型灯阵亮，不应存在该情况，仅debug用
			ARM_Inside_ligthting_effect(1, CONVEYER_BELT, 3, BUFF_COLOR);
			
		if((arm_rectangle_on>>5)&0x01 && (arm_Utype_on>>5)&0x01)//全亮
			ARM_Outside_ligthting_effect(2, HITTED, BUFF_COLOR);
		else if((arm_rectangle_on>>5)&0x01 && !((arm_Utype_on>>5)&0x01))//矩形灯阵亮
			ARM_Outside_ligthting_effect(2, WAIT_HIT, BUFF_COLOR);
		else if(!((arm_rectangle_on>>5)&0x01) && !((arm_Utype_on>>5)&0x01))//全灭
			ARM_Outside_ligthting_effect(2, UNSETLECTED, BUFF_COLOR);
		else if(!((arm_rectangle_on>>5)&0x01) && ((arm_Utype_on>>5)&0x01))//U型灯阵亮，不应存在该情况，仅debug用
			return_data = ARM_Inside_ligthting_effect(2, CONVEYER_BELT, 3, BUFF_COLOR);
	#endif
}

void buff_all_on(void)//判断哪些臂已完成击打，设置为全亮
{
	#ifdef MASTER_CONTROL//主控
		if((last_arm_flash>>0)&0x01)//判断是否刷新全亮
			return_data = ARM_Inside_ligthting_effect(0, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>1)&0x01)
			return_data = ARM_Inside_ligthting_effect(1, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>2)&0x01)
			return_data = ARM_Inside_ligthting_effect(2, ALL_ON, 0, BUFF_COLOR);
	#endif
	#ifdef SECONDARY_CONTROL//副控
		if((last_arm_flash>>3)&0x01)//判断是否刷新全亮
			return_data = ARM_Inside_ligthting_effect(0, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>4)&0x01)
			return_data = ARM_Inside_ligthting_effect(1, ALL_ON, 0, BUFF_COLOR);
		if((last_arm_flash>>5)&0x01)
			return_data = ARM_Inside_ligthting_effect(2, ALL_ON, 0, BUFF_COLOR);
	#endif
}

uint8_t buff_sucess_process_var(void)//大符全部击打成功灯效
{
	#ifdef MASTER_CONTROL//主控
		ARM_Inside_ligthting_effect(0, PROGRESS_BAR_1, 4, BUFF_COLOR);
		ARM_Inside_ligthting_effect(1, PROGRESS_BAR_1, 4, BUFF_COLOR);
		return ARM_Inside_ligthting_effect(2, PROGRESS_BAR_1, 4, BUFF_COLOR);
	#endif
	#ifdef SECONDARY_CONTROL//副控
		ARM_Inside_ligthting_effect(0, PROGRESS_BAR_1, 4, BUFF_COLOR);
		ARM_Inside_ligthting_effect(1, PROGRESS_BAR_1, 4, BUFF_COLOR);
		return ARM_Inside_ligthting_effect(2, PROGRESS_BAR_1, 4, BUFF_COLOR);
	#endif
	#ifdef THIRD_CONTROL//副控
		ARM_Outside_ligthting_effect(0, SUCCESS, BUFF_COLOR);
		ARM_Outside_ligthting_effect(1, SUCCESS, BUFF_COLOR);
		ARM_Outside_ligthting_effect(2, SUCCESS, BUFF_COLOR);
	#endif
	#ifdef FOURTH_CONTROL//副控
		ARM_Outside_ligthting_effect(0, SUCCESS, BUFF_COLOR);
		ARM_Outside_ligthting_effect(1, SUCCESS, BUFF_COLOR);
		ARM_Outside_ligthting_effect(2, SUCCESS, BUFF_COLOR);
	#endif
	
}

void buff_reset(void)//大符初始化
{
	clear_with_purity_color(0);//整个大符清屏
	#ifdef MASTER_CONTROL//主控
		hit[0] = 1;//开启第一个臂
		secondary_finished_flag = 0;//副控成功打符灯效完成标志清零
	#endif
	arm_flash = 0;
	arm_flashed = 0;
	last_arm_flash = 0;
	arm_rectangle_on = 0;
	arm_Utype_on = 0;
}

void buff_flag_sucess(void)//打符成功标志位处理
{
	arm_flash = 0xff;
	arm_flashed = 0xff;
	last_arm_flash = 0xff;
}
#ifdef MASTER_CONTROL//主控
void buff_new_armnum_produce(void)//设置需要刷新的臂
{
	if(arm_flashed == 0x1f)/*已全部被刷新过*/
	{
		memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));//起始index清零，准备成功打符灯效，副控使用标志flag_secondary判断是否为首次进行成功打符灯效判断是否要进行index清零
		buff_flag_sucess();
	}else//还有未被刷新的臂
	{
		last_arm_flash = arm_flash;//保存上次更新的臂标号
		do arm_flash = 0x01<<rand()%5;//随机生成下一个目标臂标号
		while((arm_flash&arm_flashed) != 0x00);//目标臂已经被刷新过则重新生成
		arm_flashed |= arm_flash;//更新已被刷新过的臂
		arm_rectangle_on |= arm_flash;//此刻刷新的臂装甲板灯条亮
	}
}
#endif
void buff_flash(void)//大符刷新函数，线程中周期调用
{
#ifdef MASTER_CONTROL//主控
	exti_time--;//装甲板检测倒计时
	if(exti_time<0)exti_time = 0;
	if(arm_flash!=0xff && hit[arm_flash])//正确装甲板被击打
	{
		arm_Utype_on |= arm_flash;//此刻arm_flash还未被刷新，被击打的臂臂灯条亮
		buff_new_armnum_produce();//设置需要刷新的臂
		memset(hit, 0, 17);//装甲板击打数据清零
	}
#endif
	if(arm_flashed == 0xff && arm_flash == 0xff && last_arm_flash == 0xff)//打符成功
	{
		#ifdef MASTER_CONTROL
		if(buff_sucess_process_var() == 0)//主控进度条完成，等待副控完成
			if(secondary_finished_flag == 1)//副控进度条完成，大符初始化
				buff_reset();
		#endif
		#ifdef SECONDARY_CONTROL//副控
			if(flag_secondary == 0)//判断是否为首次进行成功打符灯效判断是否要进行index清零,准备成功打符灯效
				memset(RGB_Start_index, 0x00, sizeof(RGB_Start_index));
			flag_secondary = 1;//下次便不需要进行start_index清零
			if(buff_sucess_process_var()==0 && flag_secondary<=3)//副控进度条完成并且首次进行初始化(副控前两次初始化向主控发送进度条完成的消息，之后不再发送)
			{
				buff_reset();
				flag_secondary++;//副控专属，首次进入打符成功灯效模式标志
				can_buffer[0] = 1;
				can_send_msg(&hcan1, 0x201, can_buffer);//给主控发信息
			}
		#endif
		#ifdef THIRD_CONTROL//副控
			buff_sucess_process_var();
		#endif
		#ifdef FOURTH_CONTROL//副控
			buff_sucess_process_var();
		#endif
	}else
	{
		#ifdef SECONDARY_CONTROL//副控
			flag_secondary = 0;
		#endif
		buff_conveyer_belt();//根据标志位选择臂进行传送带灯效显示
		buff_all_on();//根据标志位选择臂进行全亮显示
	}
	SMD_LED_PWM_Init();
}

