/** 
* @file         Can_Driver.cpp 
* @brief        CAN_Driver常用执行驱动
* @details      robomaster常用的执行驱动
* @author      WMD
* @date     2018年10月10日17:37:37
* @version  1.0
* @par Copyright (c):  
*       WMD 
* @par 日志
*      2018年10月10日17:37:39 针对新的cube固件库做了一次兼容性更新
*      2018年11月7日20:10:11 不再将宏定义放在此处，而用错误检测替代之 建议使用什么CAN放在can.h
*/  
#include "Can_Driver.hpp"
#include "can.h"//!<当Cube设置“为每个头文件单独生成文件”时才会有这个文件，如果找不到该文件请在cube中设置
//CAN的初始化函数
#define ABS(x) ((x)>0?(x):-(x))
#define ERROR_STACK_SIZE 20

#ifndef USE_CAN1
#ifndef USE_CAN2
#error Not selected any CAN to init!Please define USE_CAN1 or USE_CAN2 to use it.
#endif
#endif

//#define BLOCK_ERROR //错误场屏蔽 在比赛时不要用
uint8_t Send_error[4]={0};//用于判断两个CAN的某处是否离线
/**
*滤波器设置，适用于HAL库1.7.3+，固件1.21.0+版本
***/
static void My_CAN_FilterConfig(CAN_HandleTypeDef* _hcan)
{
	CAN_FilterTypeDef		CAN_FilterConfigStructure;
	//滤波器设置
	CAN_FilterConfigStructure.FilterMode = CAN_FILTERMODE_IDMASK;
	CAN_FilterConfigStructure.FilterScale = CAN_FILTERSCALE_32BIT;
	CAN_FilterConfigStructure.FilterIdHigh = 0x0000;
	CAN_FilterConfigStructure.FilterIdLow = 0x0000;
	CAN_FilterConfigStructure.FilterMaskIdHigh = 0x0000;
	CAN_FilterConfigStructure.FilterMaskIdLow = 0x0000;
	CAN_FilterConfigStructure.FilterFIFOAssignment = CAN_FilterFIFO0;
	CAN_FilterConfigStructure.FilterActivation = ENABLE;
#ifdef USE_CAN1
	if(_hcan == &hcan1)
	{
        CAN_FilterConfigStructure.FilterBank = 0;
		if(HAL_CAN_ConfigFilter(_hcan, &CAN_FilterConfigStructure) != HAL_OK)
		{
			while(1);//等待
		}
	}
#endif
#ifdef USE_CAN2
	if(_hcan == &hcan2)
	{
        CAN_FilterConfigStructure.FilterBank = 14;
		if(HAL_CAN_ConfigFilter(_hcan, &CAN_FilterConfigStructure) != HAL_OK)
		{
			while(1);//等待
		}
	}
#endif
	
}

//记得 如果需要改变CAN的数量 这里一定要再改
void CAN_Init_All(void)
{
#ifdef USE_CAN1
	  My_CAN_FilterConfig(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1,CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_CAN_Start(&hcan1);
#endif
#ifdef USE_CAN2
	  My_CAN_FilterConfig(&hcan2);
    HAL_CAN_ActivateNotification(&hcan2,CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_CAN_Start(&hcan2);
#endif
}
/****
*@func  通用型can发送函数
*@brief 确定id 和发送数据 把数据发送出去 注意此处入口参数是4个int16_t型数据 和8个uint8_t不一样
*@Para
*@Retal
*@data
*******/
HAL_StatusTypeDef can_send_msg(CAN_HandleTypeDef* _hcan, int id, int16_t* s16buff)
{
	///错误屏蔽
#ifdef BLOCK_ERROR
		if(_hcan==&hcan1 && id==0x200 && Send_error[0]>50)return HAL_ERROR;
	  else if(_hcan==&hcan1 && id==0x1ff && Send_error[1]>50)return HAL_ERROR;
		else if(_hcan==&hcan2 && id==0x200 && Send_error[2]>50)return HAL_ERROR;
		else if(_hcan==&hcan2 && id==0x1ff && Send_error[3]>50)return HAL_ERROR;
#endif
    CAN_TxHeaderTypeDef	TxHead;//!规定发送的帧格式
		TxHead.RTR=CAN_RTR_DATA;//标准RTR
    TxHead.IDE=CAN_ID_STD;//标准ID不扩展
		TxHead.DLC = 8;
		TxHead.StdId =id;
    uint8_t Data[8];//中间数组，用于高低位更换
		Data[0] = (uint8_t)((*(s16buff+0)>>8));
		Data[1] = (uint8_t)(*(s16buff+0));
		Data[2] = (uint8_t)((*(s16buff+1)>>8));
		Data[3] = (uint8_t)(*(s16buff+1));
		Data[4] = (uint8_t)((*(s16buff+2)>>8));
		Data[5] = (uint8_t)(*(s16buff+2));
		Data[6] = (uint8_t)((*(s16buff+3)>>8));
		Data[7] = (uint8_t)(*(s16buff+3));
    uint32_t FifoLevel;//当前发送队列的长度
    HAL_StatusTypeDef result=(HAL_CAN_AddTxMessage(_hcan,&TxHead,Data,&FifoLevel));
#ifdef BLOCK_ERROR
	if(result == HAL_TIMEOUT)//异常处理
	{
		if(_hcan==&hcan1 && id==0x200)
		{
			Send_error[0]++;
		}else if(_hcan==&hcan1 && id==0x1ff)
		{
			Send_error[1]++;
		}else if(_hcan==&hcan2 && id==0x200)
		{
			Send_error[2]++;
		}else if(_hcan==&hcan2 && id==0x1ff)
		{
			Send_error[3]++;
		}
	}
#endif
	return result;
	
}
HAL_StatusTypeDef CAN_SEND_PROTECT(CAN_HandleTypeDef *_hcan,int id)
{
    CAN_TxHeaderTypeDef	TxHead;//!规定发送的帧格式
		TxHead.RTR=CAN_RTR_DATA;//标准RTR
    TxHead.IDE=CAN_ID_STD;//标准ID不扩展
		TxHead.DLC = 8;
		TxHead.StdId =id;
    uint8_t Data[8]={0};
    uint32_t FifoLevel;//当前发送队列的长度
    return (HAL_CAN_AddTxMessage(_hcan,&TxHead,Data,&FifoLevel));
}




