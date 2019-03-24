/** 
	* 其实个人感觉是个超级牛力DJI特级驱动包
	* @file         Car_Driver.cpp 
	* @brief        WMD模块化散装库————————大疆目前能用到的大部分电机实用模块Cpp版本（HAL限定） 
	* @details  
	* 初始化：创造想用的类的对象 CAN接收回调上放上CANUpdate() 在周期调用CANSend() \n
	* 使用:类里面的操作函数直接用 \n
	* @author      WMD,Onion rain
	* @date     2018年6月8日20:20:52
	* @version  5.0
	* @par Copyright (c):  
	*       WMD,Onion rain
	* @par 日志

	*		V6.0  从motor类中提出电机数据更新和发送部分作为manager类，将motor变为manager子类，cloud类也作为manager子类 \n
	*		V6.1  云台电机新增位置差获取速度近似值(然而并不好用) \n
	*		V6.2  motor Angle_Set形参改为float型，单位为° \n
	*		V6.3  新增Motor_t结构体，记录电机(电调)参数，初始化电机类时将指针传入电机类 \n
	*		V6.4  pid类新增子类：非线性pid \n
	*		V6.5  新增softcloud类，6020电机专属 \n
	*		V6.6  motor类及其子类可选电流环pid \n
	*		V6.7a Motor_t类型增加快速初始化列表 \n
	*		V6.7b motor、cloud类增加RealAngle参数表示电机反馈位置换算得到的角度，单位为°，在各自类内的update更新 \n
	*		V6.8a 整改CANSend函数，不再限定Position_Run()和Speed_Run()，使用Handle()函数自行管理发送逻辑
	*		V6.8b 整改block_type类，不再限定于softmotor类型，并拥有更自动化的方式
	*		V6.9a pid类Dout改为输出D，使用Dout_Accumulative进行累计
	*		V6.9b 云台初始化时检测外环pid存在则使用陀螺仪角速度作为外环微分值
	*		V6.9c 修复了softcloud类Angle_Set机械角模式归零位疯狂转头的bug
	*		V6.9d 增加了底盘控制类chassiscontrol，便于云台与底盘采样板通信
	*		V6.10 修复了pid类中start_time导致积分异常的bug
	*		V6.10a 真正修复了pid类中start_time导致积分异常的bug
	*		V6.11  修复了Block_Type更新后不能工作的问题
	*		V6.12  移除电机中自带的电流环，同时在底盘类型中提供了电流环控制，并提供了功率控制系统，使用时需要在CarDrv_Config.hpp中添加 \n 
						 功率限制LIMIT_P宏定义， 不需要限制功率的话不提供电流环即可.同时添加了@ref manager::UserProcess()中间处理函数，允许 \n
						 用户在pid运行后发送前添加自定义处理逻辑，同时还允许用户将已定义的电机作为一个非独立电机通过参数 @ref manager::cooperative \n
						 。设定该参数后@ref manager::CANSend() 将不会自动运行该电机的pid，需要用户在@ref manager::UserProcess()自行处理pid.
*/  
#include "Car_Driver.hpp"
#include "string.h"
#include "Can_Driver.hpp"
#include "can.h"
#include "math.h"

#define WEAK __attribute__((weak)) //使用WEAK类型是方便特殊电机来重构特定函数

using namespace std;
/** 
*   @brief 电机库错误码
*         Bit [0]:电机状态超出预料
*/
uint8_t Error_Code=0x00;

////*******************************************传统PID类*************************************************************************////
/** 
	* @brief  传统pid构造函数
	* @param [in]   P 比例系数
	* @param [in]	 I 积分系数
	* @param [in]	 D 微分系数
	* @param [in]	 IMax 积分限幅
	* @param [in]	 PIDMax 输出限幅
	* @param [in]	 T 积分时间
	* @param [in]	 I_Limited 进行积分调节的误差区间限制
	* @par 日志 
	*
*/
WEAK pid::pid(float P, float I, float D, float IMax, float PIDMax, uint16_t T, uint16_t I_Limited)
{
	this->P=P;
	this->I=I;
	this->D=D;
	this->IMax=IMax;
	this->PIDMax=PIDMax;
	this->T=T;
	this->I_Limited=I_Limited;
}
/** 
	* @brief  非线性pid构造函数
	* @param [in]   Kp ∈(ap,ap+bp),err=0时Kp最小,bp为变化区间,cp调整Kp变化速率,偶函数开口向上
	* @param [in]   Ki ∈(0,ai),err=0时ki最大,ci调整Ki变化速率,偶函数开口向下
	* @param [in]   Kd ∈(ad,ad+bd),err=0时Kd=ad+bd/(1+cd),dd调整Kd变化速率,单调递减
	* @param [in]	 IMax 积分限幅
	* @param [in]	 PIDMax 输出限幅
	* @param [in]	 T 积分时间
	* @param [in]	 I_Limited 进行积分调节的误差区间限制
	* @par 日志 
	*
*/
pid::pid(float ap, float bp, float cp,
				 float ai,           float ci,
				 float ad, float bd, float cd, float dd,
				 float IMax, float PIDMax, uint16_t T, uint16_t I_Limited)
{
	this->ap = ap; this->bp = bp; this->cp = cp;
	this->ai = ai;                this->ci = ci;
	this->ad = ad; this->bd = bd; this->cd = cd; this->dd = dd;
	this->IMax=IMax;
	this->PIDMax=PIDMax;
	this->T=T;
	this->I_Limited=I_Limited;
}
/** 
	* @brief  pid运行函数
	* @param [in]   err 传入pid环的误差 
	* @retval  pid的运行输出 
	* @par 日志 
	*
*/
WEAK float pid::pid_run(float err)
{
	CurrentError = err;
	Pout = CurrentError*P;
	
	if(ABS(CurrentError) < I_Limited)//仅在小于误差区间时进行I积分
		Iout += I	*	CurrentError;
	else Iout=0;
	
	if(Custom_Diff!=NULL)//存在自定义微分数据
		Dout_Accumulative+=(*Custom_Diff)*D;
	else
		Dout_Accumulative+=(CurrentError - LastError)*D;
	
	if(HAL_GetTick() - start_time > T)//如果达到了时间区间的话则进行微分输出
	{
		Dout=Dout_Accumulative;
		Dout_Accumulative = 0;
		start_time = HAL_GetTick();//重新定义微分开始时间
	}
	
	if(Iout	>=	IMax)Iout=IMax;
	if((Iout)	<=	-(IMax))Iout=-(IMax);
	
	PIDout = Pout + Iout + Dout;
	if(PIDout	>=	PIDMax)PIDout = PIDMax;
	if(PIDout	<=	-(PIDMax))PIDout = -(PIDMax);
	
	LastError=CurrentError;
	return PIDout;
}
/** 
	* @brief  非线性pid运行函数
	* @param [in]   err 传入pid环的误差 
	* @retval  pid的运行输出 
	* @par 日志 
	*
*/
float pid::nonlinear_pid_run(float err)
{
	P = ap + bp * (1 - sech(cp * err));
	I = ai * sech(ci * err);
	
	//非线性pid增益调节参数曲线——kd曲线为单调递减函数，貌似只支持一种变化方向，此处处理保持输入kd(err)的err>0
	if(err > 0)
		D = ad + bd / (1 + cd * exp(dd * err));
	else 
		D = ad + bd / (1 + cd * exp(dd * (-err)));
	
	pid_run(err);
	return PIDout;
}
float pid::sech(float in)
{
	return 1/cosh(in);
}
////*******************************************block_type类******************************************************************////
/** 
	* @brief  初始化堵转检测
	* @param [in]	Limit 电流堵转阈值
	* @param [in]	 time 堵转触发时间
	* @param [in]	 err_num 在多少范围内认为是堵转
	* @par 日志 
    *       2019年1月19日10:41:56 重构block_type使内部更简洁，接口更方便，功能更全面
	*		2019年1月24日14:46:09 更改该接口变量形式
	*
*/
void block_type::Block_Init(uint16_t Limit, uint16_t time, float err_num)
{
	block_Current_Limit=Limit;
	block_Time_Limit=time;
	block_err_num=err_num;
}
/** 
	* @brief  堵转检测 该函数会由电机托管周期执行
	* @retval  1 堵转
	* @retval  0 未堵转
	* @par 日志 
	*
*/
void block_type::Block_Check(void)
{
	if(ABS(Current)>this->block_Current_Limit)//电流超过阈值被判定为堵转
	{
		if(block_flag==0)//没有登记堵转
		{
			block_flag=1;
			block_Angle=RealAngle;//登记当前堵转的位置
			block_time=HAL_GetTick();//登记当前堵转时间
		}else//已经登记堵转 判断是否持续
		{
			if(ABS(block_Angle-RealAngle) < block_err_num)//登记堵转时刻至此刻间仍未转过阈值角度
			{
				if(HAL_GetTick()-block_time > block_Time_Limit)//堵转时间大于阈值时间
				{
					IsBlock=1;
					return;
				}
			}else block_flag=0;//已经变化了 不再堵转了
		}
	}else block_flag=0;
}
/** 
* @brief  清除堵转标志位
* @details 重新复位堵转检测变量，清除堵转标志
* @retval  OK  成功 
* @retval  ERROR   错误  
* @par 日志 
*
*/
void block_type::Clear_BlockFlag(void)
{
	block_flag=0;
	IsBlock=0;
}
////*******************************************manager类*********************************************************************////
/** 
	* @brief 管家类的静态变量,用于存储电机对应id的待发送值,当前电流值优先级,以及其本身指针
*/
CAN_HandleTypeDef* manager::CanHandle1;
CAN_HandleTypeDef* manager::CanHandle2;
manager* manager::CAN1MotorList[8]={NULL};
int16_t manager::CAN1CurrentList[8]={0};
int8_t manager::CAN1_OnlineID;
manager* manager::CAN2MotorList[8]={NULL};
int16_t manager::CAN2CurrentList[8]={0};
int8_t manager::CAN2_OnlineID;
/** 
	* @brief  确认选择使用的CAN口
	* @param [in]   canhandle1 CAN的句柄 一般为hcan1或hcan2
	* @param [in]   canhandle2 CAN的句柄 一般为hcan1或hcan2
	* @par 日志 
	*       2018年10月31日17:11:18 加入日志
*/
void manager::CANSelect(CAN_HandleTypeDef* canhandle1, CAN_HandleTypeDef* canhandle2)
{
	CanHandle1=canhandle1;
	CanHandle2=canhandle2;
}
/** 
	* @brief  检测当前电机是否离线
	* @retval  1  凉了，离线了
	* @retval  0  在线着呢
	* @par 日志 
	*
*/
uint8_t manager::Is_Offline(void)
{
	if(can_code/10==1)//为CAN1电机
			return !(CAN1_OnlineID & (0x01<<(can_code%10)));
	else//为CAN2电机
			return !(CAN2_OnlineID & (0x01<<(can_code%10)));
}
/**
	* @brief  电机数据更新总管程序,负责所有电机的数据接收
	* @param [in]   _hcan 进入该中断的CAN
	* @param [in]   RxHead 本次数据格式结构体指针
	* @param [in]   Data 数据内容指针
	* @retval  OK  成功
	* @retval  ERROR   错误
	* @par 日志 
	*      2018年10月10日17:24:53 加入新固件库的支持
*/
WEAK void manager::CANUpdate(CAN_HandleTypeDef* _hcan, CAN_RxHeaderTypeDef* RxHead,uint8_t* Data)
{
	uint16_t id=RxHead->StdId;
	if(_hcan==CanHandle1)
	{
		if(id<=0x208 && id>=0x201)//是标准的大疆数据帧
			if(CAN1MotorList[id-0x201]!=NULL)
				CAN1MotorList[id-0x201]->update(Data);
	}
	if(_hcan==CanHandle2)
		if(id<=0x208 && id>=0x201)//是标准的大疆数据帧
			if(CAN2MotorList[id-0x201]!=NULL)
				CAN2MotorList[id-0x201]->update(Data);
}
/** 
* @brief  CAN总线上的自定义数据处理函数
* @par 日志 
*       2019年3月8日16:12:26 wmd 该函数被创建
*/
WEAK void manager::UserProcess(void)
{
    //该函数本身不执行任何操作，需要在PID跑完，发送之前执行代码的话就在自己的文件重写该函数即可
    UNUSED(UserProcess);
}
/** 
	* @brief  CAN发送总管.所有的CAN电机都会在这里进行处理，发送
	* @retval  0 成功
	* @retval  第一位置1 CAN1 0x200发送失败
	* @retval  第二位置1 0x1ff发送失败
	* @retval  第三位置1 0x200发送失败
	* @retval  第四位置1 0x1ff发送失败
	* @par 日志
	*       2018年10月31日16:46:48 处理逻辑加深
	*       2018年11月1日20:54:09  加入在线电机管理，离线电机不发送的功能在此处实现
	*       2019年1月19日14:34:55  处理逻辑进一步优化，每一个电机不局限于位置和速度两种， 可以自由增加功能，不再需要在CANSend里勉强了 
    *       2019年3月14日15:09:13 加入了合作类型电机，合作类型电机不在CANSend中自动处理，要求这类电机应在UserHandle中处理
*/
uint8_t manager::CANSend(void)
{
	if(CanHandle1!=NULL)//结算CAN1数据
	{
		for(uint8_t i=0;i<8;i++)//对8个电机进行轮询
		{
			if(CAN1MotorList[i]!=NULL)//如果这个电机存在
			{
				if(CAN1MotorList[i]->cooperative==0)CAN1MotorList[i]->Handle();//如果不是协作型电机，就执行其单独程序
				if(HAL_GetTick() - CAN1MotorList[i]->LastUpdateTime > 10)//如果电机的数据已经超过10ms没有更新了
				{
					CAN1_OnlineID &= ~(0x01<<i);//对相应online位置0
				}else
				{//以下为在线情况
					CAN1_OnlineID |=0x01<<i;//对相应online位置1
				}
			}
		}
	}
	if(CanHandle2!=NULL)//结算CAN2数据
	{
		for(uint8_t i=0;i<8;i++)//对8个电机进行轮询
		{
			if(CAN2MotorList[i]!=NULL)//如果这个电机存在
			{
				if(CAN2MotorList[i]->cooperative==0)CAN2MotorList[i]->Handle();//如果不是协作型电机，就执行其单独程序
				if(HAL_GetTick() - CAN2MotorList[i]->LastUpdateTime > 10)//如果电机的数据已经超过10ms没有更新了
				{
					CAN2_OnlineID &= ~(0x01<<i);//对相应online位置0
				}else
				{//以下为在线情况
					CAN2_OnlineID |=0x01<<i;//对相应online位置1
				}
			}
		}
	}
	if(chassis::point!=NULL)chassis::point->Handle();//如果存在底盘的话则进行底盘的功率控制处理
	UserProcess();//进行用户的自定义数据处理
	//以下为发送处理
	uint8_t check=0;
	uint8_t result=0;
	if((CAN1_OnlineID&0x0f)!=0)//判断该列中有没有在线电机
		check=can_send_msg(CanHandle1,0x200,(int16_t*)CAN1CurrentList);
	if(check)result|=0x01;
	if((CAN1_OnlineID&0xf0)!=0)//判断该列中有没有在线电机
		can_send_msg(CanHandle1,0x1ff,(int16_t*)&CAN1CurrentList[4]);
	if(check)result|=0x02;
	if((CAN2_OnlineID&0x0f)!=0)//判断该列中有没有在线电机
		can_send_msg(CanHandle2,0x200,(int16_t*)CAN2CurrentList);
	if(check)result|=0x04;
	if((CAN2_OnlineID&0xf0)!=0)//判断该列中有没有在线电机
		can_send_msg(CanHandle2,0x1ff,(int16_t*)&CAN2CurrentList[4]);
	if(check)result|=0x08;
	memset(CAN1CurrentList,0,16);
	memset(CAN2CurrentList,0,16);
	return result;
}
WEAK void manager::Speed_F_Set(float f)///设定前馈量
{
	Speed_F=f;
}
////*******************************************motor类***********************************************************************////
/** 
	* @brief  电机类构建函数
	* @param [in]  can_num	第几个CAN 只允许输入1或2
	* @param [in]  _can_id		该电机的CAN_ID,如:0x201
	* @param [in]  *motor_type 电机类型结构体指针
	* @param [in]  _PID_In	内环PID 输入pid对象指针
	* @param [in]	 _PID_Out 外环PID	输入pid对象指针 
	* @param [in]	 _PID_Current 电流环PID	输入pid对象指针 
	* @param [in]	 CurrentSource 采样板电流采样数据
	* @par 日志 
	*
*/
WEAK motor::motor(const uint8_t can_num, const uint16_t _can_id, Motor_t *motor_type, pid *_PID_In, pid *_PID_Out)
	:MotorType(motor_type), PID_In(_PID_In), PID_Out(_PID_Out)
{
	//在motor对象表里面加入自己的指针
	if(can_num==1)CAN1MotorList[_can_id-0x201]=this;
	if(can_num==2)CAN2MotorList[_can_id-0x201]=this;
	can_code=can_num*10+_can_id-0x201;
}
/** 
	* @brief  电机中断更新数据的函数
	* @param [in]   Data[] 中断时接到的数据
	* @par 日志 
	*       2018年10月31日19:19:56 整合两个update函数为一个update函数
*/
WEAK void motor::update(uint8_t Data[])
{
	this->LastSpeed=this->RealSpeed;
	this->LastPosition=this->RealPosition;
	this->RealPosition=Data[0]<<8 | Data[1];
	this->RealSpeed=Data[2]<<8 | Data[3];
	LastUpdateTime=HAL_GetTick();//更新本次电机数据最后更新的时间
	RealAngle = RealPosition*360.f/MotorType->max_mechanical_position;//根据机械角计算出的真实角度
}
/** 
* @brief  电机进行数据处理的函数 该函数会在CAN发送前执行，一般用于判断运行状态，运行PID计算电流
* @par 日志 
*
*/
WEAK void motor::Handle(void)
{
	if(block!=NULL && RunState!=Stop)
	{
		block->Block_Check();
	}
	switch(RunState)
	{
		case Speed_Ctl://电机在以速度态运行
			Speed_Run();
			break;
		case Position_Ctl://路程
			Position_Run();
			break;
		case Stop://停止状态
			Safe_Set();
			break;
		default:
			Error_Code|=0x01;//生成错误码
			Safe_Set();
			break;
	}
}
/** 
	* @brief  对该电机执行安全指令
	* @par 日志 
	*
*/
WEAK void motor::Safe_Set(void)
{
	if(block!=NULL)block->IsBlock=0;//去除堵转标志，避免在逻辑中依然认为是堵转
	RunState = Stop;
	TargetCurrent = 0;
	InsertCurrent();
}
/** 
	* @brief  电机的位置给定值
	* @note 该函数为设置电机转子转动一圈内的位置
	* @param [in]   Target_Angle 目标角度
	* @par 日志 V1.0 创建该函数
	*           V1.1 形参改为float型，表示角度，单位°
	*
*/
WEAK void motor::Angle_Set(float Target_Angle)
{
  RunState = Position_Ctl;
	TargetPosition = Target_Angle/360.f * MotorType->max_mechanical_position;
}
/** 给定电机目标速度
	* @param [in]   Speed 目标速度
	* @par 日志
	*       2018年10月31日17:31:27 创立该函数
	*
*/
WEAK void motor::Speed_Set(int16_t Speed)
{
	RunState=Speed_Ctl;
  TargetSpeed=Speed;
}
/** 
	* @brief  设定电机的位置PID
	* @warning 该函数的位置环无路程累计效应，只能用于不超过一圈的普通云台电机
	* @retval  Current 该电机此时应该发送的电流值
	* @note 此函数由motor::CANSend托管
	* @par 日志 
	*       2018年10月31日17:01:51 曾用名Angle_Set，现改为私有函数并统一运行
	*
*/
WEAK void motor::Position_Run(void)
{
	if(PID_Out==NULL)while(1);//如果没有外环pid的话直接暴露错误
	int16_t err=	TargetPosition	-	RealPosition;//此时得数误差为路程环误差
	err=PID_Out->pid_run(err);//计算后此时err为目标速度
	TargetSpeed=err;
	Speed_Run();//把目标速度放进速度控制里
}
/** 
	* @brief  设定电机速度PID
	* @note 该函数由库内部托管处理
	* @par 日志 
	*       2018年10月31日17:34:04 创建该函数
    *       2019年3月9日16:20:28 删除了对Current_Run()的继续迭代！本次改动不能够向下兼容
*/
WEAK void motor::Speed_Run(void)
{
	TargetCurrent = PID_In->pid_run(this->TargetSpeed - RealSpeed);
	if(Speed_F!=0)//使用了前馈控制
	{
		//前馈处理段
		Speed_LPF = LPF_NUM*(RealSpeed-LastSpeed) + (1-LPF_NUM)*(Speed_LPF);//速度前馈低通滤波器
		TargetCurrent = TargetCurrent + Speed_LPF * Speed_F;//此处第二项为速度环前馈
	}
    InsertCurrent();
}
/** 
	* @brief  电流环PID
	* @note 该函数由库内部托管处理
	* @par 日志 
	*       2019年1月14日 创建该函数
	*
*/
//WEAK void motor::Current_Run(void)
//{
//	if(PID_Current==NULL)while(1);//若有电流采样数据无电流环pid，暴露错误
//	if(RealSpeed<0)*RealCurrent *= -1;//采样电流符号
//	CurrentSend = PID_Current->pid_run(this->TargetCurrent - *RealCurrent);
//	InsertCurrent();
//}
/** 
	* @brief  根据优先级把要发送的电流值送入列表中等待发送
	* @par 日志 
	*
*/
WEAK void motor::InsertCurrent(void)
{
	if(can_code/10==1)
		CAN1CurrentList[can_code%10] = TargetCurrent;
	if(can_code/10==2)
		CAN2CurrentList[can_code%10] = TargetCurrent;
}
/** 
	* @brief  对该电机启用堵转检测
	* @param [in]   Limit 电流堵转阈值
	* @param [in]	 time 堵转触发时间
	* @param [in]	 err_num 在多少范围内认为是堵转
	* @retval  0  成功
	* @retval  -1 错误 可能是堆栈空间不足
	* @par 日志
	*
*/
int8_t motor::Enable_Block(uint16_t Limit, uint16_t time, uint16_t err_num)
{
	if(block==NULL)
	{
		//分配内存
		block=new block_type(TargetCurrent, RealAngle);//注意 这里的电流变量值和softmotor不一样
		if(block==NULL)return -1;
	}
	block->Block_Init(Limit, time, err_num);
	return 0;
}
////*******************************************softmotor类*******************************************************************////
/** 
	* @brief  更新电机值 包括了软路程的解算
	* @retval  none
	* @par 日志 
	*
*/
WEAK void softmotor::update(uint8_t Data[])
{
	motor::update(Data);//调用父类motor的通用update函数
	if(running_flag==0)
	{
		LastPosition=RealPosition;
		running_flag=1;
	}
	if(RealPosition	-	LastPosition	>	4096)//圈数累计
		Soft_RealPosition--;
	else if(RealPosition	-	LastPosition	<-4096)//圈数累计
		Soft_RealPosition++;
	RealAngle = /*圈数对应角度*/(Soft_RealPosition)/(1.0*MotorType->Reduction_ratio)*360 \
							+ /*单圈内角度*/1.0f*RealPosition / (8192 * MotorType->Reduction_ratio )*360;//转换为角度
}
///对软路程进行限制范围
WEAK void softmotor::Limit(float _max, float _min)
{
	max=_max;
	min=_min;
}
/** 
	* @brief   有参数的位置环确定 会使其移动到指定位置
	* @param [in]   Target_Angle 目标角度
	* @retval  电流值
	* @par 日志 
	*       V1.0 2018年10月31日19:43:18 将PID控制抽离了出来 该函数只管负责设定目标值了
	*       V1.1 形参表示角度，单位°
*/
WEAK void softmotor::Angle_Set(float Target_Angle)
{
  RunState=Position_Ctl;
	if(Target_Angle>max)Target_Angle=max;//对目标角度限幅
	if(Target_Angle<min)Target_Angle=min;
	Target_Angle=Target_Angle*MotorType->Reduction_ratio/360;//得到电机减速前目标圈数
	TargetPosition=(Target_Angle-(int32_t)Target_Angle)*MotorType->max_mechanical_position;//小数部分 减速前单圈位置
	Soft_TargetPosition=(int32_t)Target_Angle;//整数部分 即减速前圈数
}
/** 
	* @brief   无参数的位置环+速度环运算
	* @retval  电流值
	* @note   该函数重写父类函数，在cansend循环调用不可改名
	* @par 日志 
	*       2018年10月31日17:13:46 改名，该运算由motor::CANSend()托管执行
	*       2018年11月3日11:32:35  加入了路程环校验环节
*/
WEAK void softmotor::Position_Run(void)
{
  if(PID_Out==NULL)while(1);//暴露错误，你没有外环还想用位置PID?
	int32_t err=0;
	err=TargetPosition-RealPosition;     //得到单圈误差
	err+=MotorType->max_mechanical_position*(Soft_TargetPosition-Soft_RealPosition);//加上圈数位置误差
	TargetSpeed = PID_Out->pid_run(err);//位置环得到目标速度
	TargetCurrent = PID_In->pid_run(TargetSpeed-RealSpeed);
	InsertCurrent();
}
/** 
	* @brief  对该电机启用堵转检测
	* @param [in]   Limit 电流堵转阈值
	* @param [in]	 time 堵转触发时间
	* @param [in]	 err_num 在多少范围内认为是堵转
	* @retval  0  成功
	* @retval  -1 错误 可能是堆栈空间不足
	* @par 日志
	*
*/
int8_t softmotor::Enable_Block(uint16_t Limit, uint16_t time, uint16_t err_num)
{
	if(block==NULL)
	{
		//分配内存
		block=new block_type(TargetCurrent, RealAngle);
		if(block==NULL)return -1;
	}
	block->Block_Init(Limit, time, err_num);
	return 0;
}
////*******************************************Cloud类***********************************************************************////
/** 
	* @brief  云台电机构造函数 \n
	* 				陀螺仪模式仅用于陀螺仪和云台一体的时候才能用 如果不使用陀螺仪模式 可以给无效值
	* @note   右偏是减少 左偏是增加 \n
	*				  一定注意！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！云台Yaw轴电机重新安装过了之后这个一定要改！！！！
	* @param [in]   can_num	第几个CAN 只允许输入1或2
	* @param [in]   _can_id		该电机的CAN_ID,如:0x201
	* @param [in]   _CLOUD_STD 该云台在指向原点时的编码器的值
	* @param [in]   *motor_type 电机类型结构体指针
	* @param [in]   PID_I		机械角控制内环
	* @param [in]   PID_O		机械角控制外环
	* @param [in]   G_In		陀螺仪控制内环
	* @param [in]   G_Out		陀螺仪控制外环
	* @param [in]   SpeedSource	    陀螺仪速度数据源
	* @param [in]   PositionSource	陀螺仪位置数据源
	* @par 日志 
	*
*/
WEAK cloud::cloud(uint8_t can_num, uint16_t _can_id, int16_t _CLOUD_STD, Motor_t *motor_type, pid* _PID_In, pid* _PID_Out, pid* G_In, pid* G_Out, float *SpeedSource, float *PositionSource)//!<构造函数，可以指定位置或速度源
	:MotorType(motor_type), Gyro_RealSpeed(SpeedSource), Gyro_RealAngle(PositionSource)
{
	//在motor对象表里面加入自己的指针
	PID_In  = _PID_In;
	PID_Out = _PID_Out;
	if(can_num==1)CAN1MotorList[_can_id-0x201]=this;
	if(can_num==2)CAN2MotorList[_can_id-0x201]=this;
	can_code=can_num*10+_can_id-0x201;
	
	CLOUD_STD=_CLOUD_STD;
	Gyro_PID_In=G_In;
	Gyro_PID_Out=G_Out;
	
	if(Gyro_PID_Out != NULL)//当陀螺仪pid外环存在时将外环的微分值设置为该陀螺仪角速度
		Gyro_PID_Out->Custom_Diff = Gyro_RealSpeed;
}
/** 
	* @brief  cloud类型的更新函数
	* @param [in]   Data[] 中断传入的更新数据
	* @note 1.该函数会把RealPosition进行基于STD_YAW的标准移位 \n
	*       2.该函数并不能通过电机获得速度值（6623是不反馈速度的）速度取陀螺仪反馈速度 \n
	* @par 日志 
	*
*/
WEAK void cloud::update(uint8_t Data[])
{
	LastPosition=RealPosition;
	LastTorque=RealCurrent;
	LastSpeed=RealSpeed;
	RealPosition=Data[0]<<8 | Data[1];  //实际位置
	//机械角的真实值需要做一个平移 以保证真正的0位置是软件0位置
	if(RealPosition-CLOUD_STD<-4096)RealPosition = RealPosition-CLOUD_STD+8192;
	else if(RealPosition-CLOUD_STD>4096)RealPosition = RealPosition-CLOUD_STD-8192;
	else RealPosition = RealPosition-CLOUD_STD;
	RealCurrent=Data[2]<<8 | Data[3];   //实际转矩电流
	if(Gyro_RealSpeed != NULL) RealSpeed = *Gyro_RealSpeed;     //有外部数据输入速度取陀螺仪速度
	//机械角做差速度震荡好严重而且1ms周期算出来的速度也很小，一滤波没了，算了
	else RealSpeed = (LastPosition - RealPosition + LastSpeed)/2; //无外部数据输入根据位置偏移量求取角速度,并做均值滤波
	LastUpdateTime=HAL_GetTick();  //更新本次电机数据最后更新的时间
	RealAngle = RealPosition*360.f / MotorType->max_mechanical_position;//转换为角度
}
/** 
* @brief  电机进行数据处理的函数 该函数会在CAN发送前执行，一般用于判断运行状态，运行PID计算电流
* @par 日志 
*
*/
WEAK void cloud::Handle(void)
{
	switch(RunState)
	{
		case Speed_Ctl://电机在以速度态运行
			Speed_Run();
			break;
		case Position_Ctl://路程
			Position_Run();
			break;
		case Gyro_Position_Ctl://陀螺仪特定模式 注意该状态只允许cloud类型拥有
			Gyro_Position_Run();
			break;
		case Gyro_Speed_Ctl://陀螺仪特定模式 注意该状态只允许cloud类型拥有
			Gyro_Speed_Run();
			break;
		case Stop://停止状态
			Safe_Set();
			break;
		default:
			Error_Code|=0x01;//生成错误码
			Safe_Set();
			break;
	}
}
/** 
	* @brief  用于切换机械角pid
	* @par 日志 2018.12.11创建
	*
*/
WEAK void cloud::Pid_Select(pid *PID_In_Select, pid *PID_Out_Select)
{
	PID_In = PID_In_Select;
	PID_Out = PID_Out_Select;
}
/** 
	* @brief  用于切换陀螺仪pid
	* @par 日志 2018.12.11创建
	*
*/
WEAK void cloud::Gyro_Pid_Select(pid *Gyro_PID_In_Select, pid *Gyro_PID_Out_Select)
{
	Gyro_PID_In = Gyro_PID_In_Select;
	Gyro_PID_Out = Gyro_PID_Out_Select;
}
/** 给定电机目标速度(机械角)
	* @param [in]   Speed 目标速度
	* @par 日志
	*
*/
WEAK void cloud::Speed_Set(float Speed)
{
	RunState=Speed_Ctl;
	TargetSpeed=Speed;
}
/** 
	* @brief  电机的位置给定值(机械角)
	* @param [in]   Target_Angle 目标角度
	* @par 日志 
	*       V1.1 形参表示角度，单位°
	*
*/
WEAK void cloud::Angle_Set(float Target_Angle)
{
  RunState=Position_Ctl;
	TargetPosition=Target_Angle*MotorType->max_mechanical_position/360;
}
/** 
	* @brief  通过陀螺仪模式设定速度
	* @param [in]   TargetSpeed 目标速度值，该变量应来自于陀螺仪的速度值 
	* @par 日志 
*/
WEAK void cloud::Gyro_Speed_Set(float TargetSpeed)
{
	RunState=Gyro_Speed_Ctl;
	Gyro_TargetSpeed=TargetSpeed;
}
/** 
	* @brief  通过陀螺仪模式设定角度
	* @param [in]   Target_Angle 目标角度值，该变量应来自于陀螺仪的角度值 
	* @par 日志 
	*       V1.0 2018年10月31日17:22:06 更改函数接口 该函数仅用于设定数据了 不参与运算
	*
*/
WEAK void cloud::Gyro_Angle_Set(float Target_Angle)
{
	RunState=Gyro_Position_Ctl;
	Gyro_TargetAngle=Target_Angle;
}
/** 
	* @brief  设定电机速度PID(机械角)
	* @note 该函数由库内部托管处理
	* @par 日志
	     2018年10月31日17:34:04 创建该函数
	     2018/12/18增加非线性pid适配
	*
*/
void cloud::Speed_Run(void)
{
	if(PID_In->ap==0 && PID_In->bp==0)//内环pid为传统pid
		TargetCurrent = PID_In->pid_run(TargetSpeed - RealSpeed);             //传统pid计算
	else TargetCurrent = PID_In->nonlinear_pid_run(TargetSpeed - RealSpeed);//非线性pid计算
	if(Speed_F!=0)//使用了前馈控制
	{
		//前馈处理段
		Speed_LPF = LPF_NUM*(RealSpeed-LastSpeed) + (1-LPF_NUM)*(Speed_LPF);//速度前馈低通滤波器
		TargetCurrent = TargetCurrent + Speed_LPF * Speed_F;                //此处第二项为速度环前馈
	}
	InsertCurrent();
}
/** 
	* @brief  设定电机的位置PID(机械角)
	* @warning 该函数的位置环无路程累计效应，只能用于不超过一圈的普通云台电机
	* @retval  Current 该电机此时应该发送的电流值
	* @note 此函数由manager::CANSend托管
	* @par 日志 
          2018/12/18增加非线性pid适配
	*
*/
WEAK void cloud::Position_Run(void)
{
	if(PID_Out==NULL)while(1);                    //如果没有外环pid的话直接暴露错误
	int16_t err = TargetPosition	-	RealPosition; //此时得数误差为路程环误差
	if(PID_Out->ap==0 && PID_Out->bp==0)          //外环pid为传统pid
		err=PID_Out->pid_run(err);                  //传统pid计算后此时err为目标速度
	else err = PID_Out->nonlinear_pid_run(err);   //非线性pid计算后此时err为目标速度
	TargetSpeed=err;
	Speed_Run();//把目标速度放进速度控制里
}
/** 
	* @brief  设定云台电机速度的PID运算(陀螺仪)
	* @note 该函数由CAN_Send()托管处理
	* @par 日志 
	       2018年11月27日19:44:19 新建该函数
	       2018/12/18增加非线性pid适配
*/
WEAK void cloud::Gyro_Speed_Run(void)
{
	while(this->Gyro_RealSpeed == NULL);//未指定陀螺仪速度数据源暴露错误
	
	if(Gyro_PID_In->ap==0 && Gyro_PID_In->bp==0)//内环pid为传统pid
		TargetCurrent = Gyro_PID_In->pid_run(Gyro_TargetSpeed - *Gyro_RealSpeed);             //传统pid计算
	else TargetCurrent = Gyro_PID_In->nonlinear_pid_run(Gyro_TargetSpeed - *Gyro_RealSpeed);//非线性pid计算
	if(Speed_F!=0)//使用了前馈控制
	{
		//前馈处理段
		Speed_LPF = LPF_NUM*(*Gyro_RealSpeed-Gyro_LastSpeed) + (1-LPF_NUM)*(Speed_LPF);//速度前馈低通滤波器
		TargetCurrent = TargetCurrent + Speed_LPF * Speed_F;//此处第二项为速度环前馈
	}
	InsertCurrent();
}
/** 
	* @brief  设定云台电机角度的PID运算(陀螺仪)
	* @note 该函数由CAN_Send()托管处理
	* @par 日志
          2018/12/18增加非线性pid适配
*/
WEAK void cloud::Gyro_Position_Run(void)
{
	if(Gyro_PID_Out==NULL)while(1);//如果没有外环pid的话直接暴露错误
	while(Gyro_RealAngle == NULL);//未指定陀螺仪位置数据源,暴露错误
	int16_t err = 50 * (Gyro_TargetAngle-*Gyro_RealAngle);//确定误差
	if(Gyro_PID_Out->ap==0 && Gyro_PID_Out->bp==0)  //外环pid为传统pid
		err=Gyro_PID_Out->pid_run(err);               //传统pid计算后此时err为目标速度
	else err = Gyro_PID_Out->nonlinear_pid_run(err);//非线性pid计算后此时err为目标速度
	Gyro_TargetSpeed = err;
	Gyro_Speed_Run();//把目标速度放进速度控制里
}
/** 
	* @brief  根据优先级把要发送的电流值送入列表中等待发送
	* @par 日志 
	*
*/
WEAK void cloud::InsertCurrent(void)
{
	if(can_code/10==1)
		CAN1CurrentList[can_code%10] = TargetCurrent;
	if(can_code/10==2)
		CAN2CurrentList[can_code%10] = TargetCurrent;
}
/** 
	* @brief  对该电机执行安全指令
	* @par 日志 
	*
*/
WEAK void cloud::Safe_Set(void)
{
	RunState = Stop;
	TargetCurrent=0;
	InsertCurrent();
}
////*******************************************softcloud类***********************************************************************////
/** 
	* @brief  更新电机值 包括了软路程的解算
	* @retval  none
	* @par 日志 
	*
*/
WEAK void softcloud::update(uint8_t Data[])
{
	LastPosition=RealPosition;
	LastTorque=RealCurrent;
	LastSpeed=RealSpeed;
	RealPosition=Data[0]<<8 | Data[1];  //实际位置
	//机械角的真实值需要做一个平移 以保证真正的0位置是软件0位置
	if(RealPosition-CLOUD_STD<(-MotorType->max_mechanical_position/2))
		RealPosition = RealPosition - CLOUD_STD + MotorType->max_mechanical_position;
	else if(RealPosition-CLOUD_STD>(MotorType->max_mechanical_position/2))
		RealPosition = RealPosition - CLOUD_STD - MotorType->max_mechanical_position;
	else RealPosition = RealPosition-CLOUD_STD;
	RealCurrent=Data[4]<<8 | Data[5];   //实际转矩电流
	if(Gyro_RealSpeed != NULL) RealSpeed = *Gyro_RealSpeed;     //有外部数据输入速度取陀螺仪速度
	else RealSpeed = (int16_t)(Data[2]<<8 | Data[3]);
	
	LastUpdateTime=HAL_GetTick();  //更新本次电机数据最后更新的时间
	if(running_flag==0)//处理第一次
	{
		LastPosition=RealPosition;
		running_flag=1;
	}
	if((RealPosition-LastPosition) > (MotorType->max_mechanical_position/2))//圈数累计
		Soft_RealPosition--;
	else if((RealPosition-LastPosition) < (-MotorType->max_mechanical_position/2))//圈数累计
		Soft_RealPosition++;
	RealAngle = /*圈数对应角度*/(Soft_RealPosition)/(1.0*MotorType->Reduction_ratio)*360 \
							+ /*单圈内角度*/1.0f*RealPosition / (MotorType->max_mechanical_position * MotorType->Reduction_ratio )*360;//转换为角度
}
/** 
* @brief  电机进行数据处理的函数 该函数会在CAN发送前执行，一般用于判断运行状态，运行PID计算电流
* @par 日志 
*
*/
WEAK void softcloud::Handle(void)
{
	switch(RunState)
	{
		case Speed_Ctl://电机在以速度态运行
			Speed_Run();
			break;
		case Position_Ctl://路程
			Position_Run();
			break;
		case Gyro_Position_Ctl://陀螺仪特定模式 注意该状态只允许cloud类型拥有
			Gyro_Position_Run();
			break;
		case Gyro_Speed_Ctl://陀螺仪特定模式 注意该状态只允许cloud类型拥有
			Gyro_Speed_Run();
			break;
		case Stop://停止状态
			Safe_Set();
			break;
		default:
			Error_Code|=0x01;//生成错误码
			Safe_Set();
			break;
	}
}
///对软路程进行限制范围
WEAK void softcloud::Limit(float _max, float _min)
{
	max=_max;
	min=_min;
}
/** 
	* @brief   有参数的位置环确定 会使其移动到指定位置
	* @param [in]   Target_Angle 目标角度
	* @retval  电流值
	* @par 日志 
*/
WEAK void softcloud::Angle_Set(float Target_Angle)
{
  RunState=Position_Ctl;
	if(Target_Angle>max)Target_Angle=max;//对目标角度限幅
	if(Target_Angle<min)Target_Angle=min;
	Target_Angle=Target_Angle*MotorType->Reduction_ratio/360;//得到电机减速前目标圈数
	TargetPosition=(Target_Angle-(int32_t)Target_Angle)*MotorType->max_mechanical_position;//小数部分 减速前单圈位置
//	Soft_TargetPosition=(int32_t)Target_Angle/360.f;//整数部分 即减速前圈数
	Soft_TargetPosition=Soft_RealPosition;//由于360云台机械角模式不需要多圈，此处目标圈数置为当前圈数，防止云台转动多圈后云台疯狂回转
}
/** 
	* @brief   无参数的位置环+速度环运算
	* @retval  电流值
	* @note   该函数重写父类函数，在cansend循环调用不可改名
	* @par 日志
*/
WEAK void softcloud::Position_Run(void)
{
  if(PID_Out==NULL)while(1);//暴露错误，你没有外环还想用位置PID?
	int32_t err=0;
	err=TargetPosition-RealPosition;     //得到单圈误差
	err+=MotorType->max_mechanical_position*(Soft_TargetPosition-Soft_RealPosition);//加上圈数位置误差
	if(PID_Out->ap==0 && PID_Out->bp==0)               //外环pid为传统pid
		TargetSpeed=PID_Out->pid_run(err);               //传统pid计算得到目标速度
	else TargetSpeed = PID_Out->nonlinear_pid_run(err);//非线性pid计算后得到目标速度
	if(PID_In->ap==0 && PID_In->bp==0)                 //内环pid为传统pid
		TargetCurrent=PID_In->pid_run(TargetSpeed-RealSpeed);                //传统pid计算得到目标电流
	else TargetCurrent = PID_In->nonlinear_pid_run(TargetSpeed-RealSpeed); //非线性pid计算后得到目标电流
	InsertCurrent();
}
chassis* chassis::point;
////*******************************************全新底盘类********************************************************************////
/** 
	* @brief  底盘对象构建函数
	* @param [in]   can_num CAN序号 可选值1 2 
	* @param [in]   First_can_id 底盘电机的第一个id号 如输入0x201 则底盘为0x201,0x202,0x203,0x204
	* @param [in]   *motor_type 电机类型结构体指针
	* @param [in]   speed_pid 速度环PID指针
    * @param [in]   turnpid 转弯PID指针 可以为空
    * @param [in]   current_pid 电流环PID指针 可以为空 为空则表示该车不限功率
    * @param [in]   CurrentSource 电流数据来源 可以为空 同上
	* @retval  OK  成功 
	* @retval  ERROR   错误  
	* @par 日志 
	*
*/
WEAK chassis::chassis(uint8_t can_num, uint16_t First_can_id, Motor_t *motor_type, pid *speed_pid, pid* turnpid, pid *current_pid, int16_t *CurrentSource):Turn_PID(turnpid)
{
	if(point==NULL)point=this;
	else while(1);//走到这里说明您声明了2个或以上的底盘对象了！
	//填充底盘的轮子和pid的vector 填充4个
	for(uint8_t i=0;i<4;i++)
	{
		Pid_spe[i]=new pid(*speed_pid);
		Pid_current[i]=new pid(*current_pid);
		this->CurrentSource[i]=CurrentSource+i;
		Motor[i]=new softmotor(can_num, First_can_id+i, motor_type, Pid_spe[i], NULL);
		//Motor[i]->cooperative=1;//声明该电机为底盘机构的一部分，不使用单独的发送函数
	}
}
/** 
	* @brief  底盘运动控制函数
	* @param [in]   Vx x轴运动速度
	* @param [in]   Vy y轴运动速度
	* @param [in]   Omega 旋转速度
	* @par 日志 
	*
*/
WEAK void chassis::Run(float Vx, float Vy, float Omega)
{
	Last_Vx=Vx;
	Last_Vy=Vy;
	Last_Omega=Omega;

	float proportion, MaxSpeed,Buffer[4];
	int16_t Speed[4];
	uint8_t index;
	
	//长方形麦轮底盘的逆运动学模型
	Buffer[0] = (Vx + Vy + Omega);
	Buffer[1] = (Vx - Vy - Omega);
	Buffer[2] = (Vx - Vy + Omega);
	Buffer[3] = (Vx + Vy - Omega);
	
	//求设定值中的最大值
	for(index=0, MaxSpeed=0; index<4; index++)
	{
		if((Buffer[index]>0 ? Buffer[index] : -Buffer[index]) > MaxSpeed)
		{
			MaxSpeed = (Buffer[index]>0 ? Buffer[index] : -Buffer[index]);
		}
	}
	//若速度设定值超过底盘允许最大速度，则等比减小速度设定值
	if(ChassisMax < MaxSpeed)
	{
		proportion = (float)ChassisMax / MaxSpeed;
		Speed[0] = Buffer[0] * proportion;
		Speed[1] = -Buffer[1] * proportion;
		Speed[2] = Buffer[2] * proportion;
		Speed[3] = -Buffer[3] * proportion; 
	}
	else
	{
		Speed[0] =  Buffer[0];
		Speed[1] =  -Buffer[1];
		Speed[2] =  Buffer[2];
		Speed[3] =  -Buffer[3];
	}
	for(uint8_t i=0;i<4;i++)
	{
		Motor[i]->Speed_Set(Speed[i]);
	}
}
WEAK void chassis::Run(void) //不带参数的Run代表保持原速度进行
{
	this->Run(Last_Vx, Last_Vy, Last_Omega);
}
WEAK void chassis::Safe()//不是制动
{
	for(uint8_t i=0;i<4;i++)
		Motor[i]->Safe_Set();
}
/** 
* @brief  底盘的托管处理程序 建议保持和CANSend同周期执行 需要手动在UserHandle中调用
* @par 日志 
*   2019年3月9日16:48:07 该函数被创建
*   2019年3月19日15:37:36 需要在CarDriver_Config.hpp中添加LIMIT_P的定义，否则会报错
*/
WEAK void chassis::Handle()
{
	//以首电机确认车辆是否处于非停止状态或不需要限制功率
	if(Motor[0]->RunState==0 || Pid_current[0]==NULL)return;
	//注意：该默认处理函数为仅保证功率限制的功能，并且电源电压为常量，需要加入超级电容或电源变化的话需要重写
	float calcPower=0;
	uint8_t i;    
	for(i=0;i<4;i++)//按照四个轮子的功率计算需要受限的功率
	{
	    calcPower+=24.0f*ABS(point->Motor[i]->TargetCurrent)/819.2f;
	}     
	if(calcPower>LIMIT_P)//理论功率已经超额
	{
	     for(i=0;i<4;i++)Motor[i]->TargetCurrent*=LIMIT_P/calcPower;//总体四轮压限
	}
	int8_t flag=1;//电流的正负标志 1为正 -1位负
	for(i=0;i<4;i++)
	{
		if(Motor[i]->RealSpeed<0)flag=-1;
		else flag=1;
		Motor[i]->TargetCurrent=Pid_current[i]->pid_run(Motor[i]->TargetCurrent/819.2f*1000.0f-*CurrentSource[i]*flag);
		Motor[i]->InsertCurrent();
	}
}
////*******************************************底盘控制类for云台********************************************************************////
/** 
	* @brief  底盘控制类对象构建函数
	* @param [in]   can_num CAN序号 可选值1 2 
	* @param [in]   Chassis_id 底盘id
	* @retval  OK  成功 
	* @retval  ERROR   错误  
	* @par 日志 
	*
*/
WEAK chassiscontrol::chassiscontrol(CAN_HandleTypeDef* canhandle, uint16_t chassis_id)
{
	Canhandle = canhandle;
	Chassis_ID = chassis_id;
}
/** 
	* @brief  底盘运动控制函数
	* @param [in]   Vx x轴运动速度,注意此处速度为int16类型
	* @param [in]   Vy y轴运动速度
	* @param [in]   Omega 旋转速度
	* @par 日志 
	*
*/
WEAK void chassiscontrol::Run(int16_t Vx, int16_t Vy, int16_t Omega, uint8_t Mode)
{
	Last_Vx=Vx;
	Last_Vy=Vy;
	Last_Omega=Omega;
	Last_Mode=Mode;
	
	int16_t sendbuf[4] = {0};
	sendbuf[0] = 0xff00 & (Mode<<8);
	sendbuf[1] = Vx;
	sendbuf[2] = Vy;
	sendbuf[3] = Omega;
	
	can_send_msg(Canhandle, Chassis_ID, sendbuf);
}
WEAK void chassiscontrol::Run(void) //不带参数的Run代表保持原速度进行
{
	this->Run(Last_Vx, Last_Vy, Last_Omega, Last_Mode);
}
WEAK void chassiscontrol::Safe()//不是制动
{
	Last_Mode=22;
	int16_t sendbuf[4] = {0};
	sendbuf[0] = 0xff00 & (Last_Mode<<8);
	can_send_msg(Canhandle, Chassis_ID, sendbuf);
}



