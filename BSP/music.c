/*
*********************************************************************************************************
									WMD模块化散装库————————高水平音乐播放器（HAL限定）
							初始化：①需要一个PWM输出来设置音频频率 还需要周期性的运行Handle()函数
				注意定时器的预分频值和各个外设的值都在宏定义里面声明了 记得改  有用到延时 注意延时的更换
									②把Music_Handle放到周期处理函数中
									③写好音乐  主函数调用Music_Play();
									2017.11.28		V1.2
*********************************************************************************************************
*/
#include "music.h"
#include "tim.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "define_all.h"

uint8_t IsDance=0;//是否在当前旋律跳舞 首先得define了DANCE

#define  R  1000000 //定时器的预分频后的1s的值

#define  ZERO     0U           //休止符
#define  MIN_ZERO 3            //短暂的休止 为了让人可以听出是两个音
#define  L1       (R/262)-1    //低1　Do
#define  half_L1 （R/277)-1    //#1　Do#
#define  L2       (R/294)-1
#define  half_L2  (R/311)-1
#define  L3       (R/330)-1
#define  L4       (R/349)-1
#define  half_L4  (R/370)-1
#define  L5       (R/392)-1
#define  half_L5  (R/410)-1
#define  L6       (R/440)-1
#define  half_L6  (R/466)-1
#define  L7       (R/494)-1

#define  M1       (R/523)-1     //中 Do
#define  half_M1  (R/554)-1
#define  M2       (R/587)-1
#define  half_M2  (R/622)-1
#define  M3       (R/659)-1
#define  M4       (R/698)-1
#define  half_M4  (R/740)-1
#define  M5       (R/784)-1
#define  half_M5  (R/831)-1
#define  M6       (R/880)-1
#define  half_M6  (R/932)-1
#define  M7       (R/988)-1

#define  H1       (R/1046)-1    //高1　Do
#define  half_H1 （R/1109)-1    //#1　Do#
#define  H2       (R/1175)-1
#define  half_H2  (R/1245)-1
#define  H3       (R/1318)-1
#define  H4       (R/1397)-1
#define  half_H4  (R/1480)-1
#define  H5       (R/1568)-1
#define  half_H5  (R/1661)-1
#define  H6       (R/1760)-1
#define  half_H6  (R/1865)-1
#define  H7       (R/1967)-1
#define  END_SONG	 1            //代表此歌已经结束

song warning[]=
{//处于意外模式的时候可以通过放这段声音来达到警示效果
	{M6,16},{0U,16},{M6,16},{0U,16},{M6,16},{0U,16},{END_SONG,1}
};
song ItsOK[]=
{
	{M1,8},{M2,8},{M3,8},{M4,8},{M5,8},{M6,8},{M7,8},{H1,8},{0U,2},
	{H5,4},{H3,8},{H1,8},{H5,4},{H3,8},{H1,8},{H5,4},{H5,4},{H5,4},{END_SONG,1}
};

song jile[]=//极乐净土!
{
  {M3,8},{M5,8},
  {M6,8},{0U,8},{0U,8},{M5,8},{M6,8},{0U,8},{0U,8},{M5,8},
  {M6,8},{H1,8},{M5,8},{M6,8},{M3,8},{0U,8},{M3,8},{M5,8},
  {M6,8},{0U,8},{0U,8},{M5,8},{M6,8},{0U,8},{0U,8},{M5,8},
  {M6,8},{H3,8},{H1,8},{H2,8},{M6,8},{0U,8},{M3,8},{M5,8},
  {M6,8},{0U,8},{0U,8},{M5,8},{M6,8},{0U,8},{0U,8},{M5,8},
  {M6,8},{H1,8},{M5,8},{M6,8},{M3,8},{M5,8},{M1,8},{M2,8},
  {M3,4},{H1,4},{M6,4},{H3,4},
  {H2,8},{H3,16},{H2,16},{H1,16},{H2,16},{M6,4},{0U,4},
	
  M6,4,M6,4,M6,4,M6,16,H1,16,H2,16,H3,16,
	M6,4,M6,4,M6,8,M5,8,M5,8,M6,8,
	M6,4,M6,4,M6,4,M6,16,H1,16,H2,16,H3,16,
	M6,4,M6,4,M6,8,H4,8,H4,8,H3,8,
	M6,4,M6,4,M6,4,M6,16,H1,16,H2,16,H3,16,
	M6,4,M6,4,M6,8,M5,8,M5,8,M6,8,
	M6,4,M6,4,M6,4,M6,16,H1,16,H2,16,H3,16,
	M6,4,M6,8,M5,8,M5,8,M6,8,M6,4,

	0U,4,0U,8,M6,8,M5,4,M5,8,M6,8,
	M5,8,M3,4,M5,16,M3,16,M3,4,0U,4,
	0U,4,0U,8,M6,4,M5,8,M6,8,M7,8,
	H1,4,M7,4,M6,8,M7,16,M6,16,M5,4,

	0U,4,0U,8,M6,8,M5,4,M6,8,M5,8,
	M5,8,M3,4,M5,16,M3,16,M3,4,0U,8,M5,8,
	M5,8,M6,8,0U,8,M5,4,M6,8,0U,8,M6,16,M7,16,
	H1,4,H2,4,M6,8,M3,4,M5,8,

	M5,8,M6,8,0U,8,H3,8,H2,4,0U,8,M6,8,
	0U,8,M6,4,H3,8,H2,8,0U,4,
	0U,8,H2,4,H1,8,H2,8,H1,16,M6,16,M6,8,M5,8,
	M5,8,M5,4,M6,4,M3,4,M5,8,

	M5,8,M6,8,0U,8,H3,8,H2,4,0U,8,M6,8,
	0U,8,M6,4,H3,8,H2,8,0U,4,
	0U,8,H2,4,H1,8,H2,8,H1,16,M6,16,M6,16,M3,16,M5,8,
	M5,8,M6,4,M5,8,M6,4,0U,8,M3,16,M5,16,

	M6,8,M6,8,M6,8,H3,8,H3,4,0U,8,M7,16,H1,16,
	H2,8,H1,16,H2,16,H2,8,H3,8,M6,4,0U,8,M5,16,M5,16,
	M6,4,H3,8,H3,16,H2,8,H1,8,M6,8,M5,8,
	M6,8,M6,16,M6,8,H1,8,H2,4,0U,4,

	M6,8,M6,16,M6,16,M6,8,M5,16,M5,16,M6,8,H3,8,0U,8,H1,8,
	H2,8,H1,16,H2,16,H2,8,H3,8,H3,4,0U,8,M5,8,
	M6,8,H1,8,0U,8,M5,8,M6,8,H3,8,0U,8,H3,16,H2,16,
	H1,8,M7,8,M6,8,M5,4,M6,4,0U,8,

	0U,8,M6,8,M5,8,M3,8,M2,8,M3,8,M1,8,L5,8,
	L6,8,L5,8,L6,8,0U,8,L6,4,0U,4,
	L6,4,0U,4,L7,4,0U,4,
	L6,16,L7,16,L6,8,L6,16,L7,16,L6,8,M1,4,0U,4,
	L7,8,M6,8,M5,8,M3,8,M2,8,M3,8,M1,8,L5,8,

	L6,8,L5,8,L6,8,0U,8,L6,4,0U,4,
	L6,4,0U,4,L7,4,0U,4,
	L6,16,L7,16,L6,8,L6,16,L7,16,L6,8,M1,4,0U,4,
	L7,4,L5,4,M6,16,M5,16,M3,16,M2,16,M1,16,L6,16,M2,16,M1,16,

	M6,8,M5,8,M6,8,M5,8,H1,8,M7,8,M6,8,M5,8,
	M5,8,M3,8,M2,8,M1,8,M3,8,M2,8,M1,16,L6,16,L5,16,L6,16,L5,4,L5,8,L3,16,L2,16,L3,4,L3,4,L2,16,L3,16,L4,16,L5,16,L6,16,L7,16,M1,16,M2,16,M2,16,M3,16,M4,16,M5,16,M6,16,M7,16,H1,16,H1,16,
	H3,8,H3,4,H3,8,H3,4,H2,8,H1,8,M7,8,H1,4,H2,8,M7,8,M5,4,M2,8,H2,4,H2,8,H1,8,M7,4,M5,4,M7,4,M7,4,M6,8,M3,8,M3,8,M5,8,
	M5,8,M6,8,0U,8,H3,8,H2,4,0U,8,M6,8,0U,8,M6,8,M6,8,H3,8,H2,4,0U,4,H1,4,H1,8,H2,8,H3,8,H5,8,H3,8,H5,8,H7,4,H6,8,H5,8,H3,8,M3,8,M3,8,M5,8,
	M5,8,M6,8,0U,8,H3,8,H2,4,0U,8,M6,8,0U,8,M6,8,M6,8,H3,8,H2,4,0U,4,0U,8,H2,8,H2,8,H1,8,H2,8,H1,16,M6,16,M6,8,M3,16,M5,16,M5,8,M6,8,M6,8,M5,8,M6,4,0U,8,M3,16,M5,16,
	M6,8,M6,8,M6,8,H3,8,H3,4,0U,8,M7,16,H1,16,H2,8,H1,16,H2,16,H2,8,H3,8,M6,4,0U,8,M5,16,M5,16,M6,4,H3,8,H3,16,H3,16,H2,8,H1,8,M6,8,M5,8,M6,8,M6,16,M6,16,M6,8,H1,8,H2,4,0U,4,
	M6,8,M6,16,M6,16,M6,8,M5,16,M5,16,M6,8,H3,8,0U,8,H1,8,H2,8,H1,16,H2,16,H2,8,H3,8,H3,4,0U,8,H3,8,H2,8,H2,8,0U,8,H1,8,H2,8,H2,8,0U,8,H3,16,H2,16,H1,8,M7,8,M6,8,M5,8,M5,8,M6,8,0U,8,M5,8,
	M6,8,H1,8,0U,8,M5,8,M6,8,H5,8,0U,8,H3,16,H2,16,H2,8,H2,8,H3,8,H5,8,H5,4,H6,4,M6,4,M6,4,M6,4,M6,16,H1,16,H2,16,H3,16,M6,4,M6,4,M6,8,M5,8,M5,8,M6,8,
	M6,4,M6,4,M6,4,M6,16,H1,16,H2,16,H3,16,M6,4,M6,4,M6,8,H4,8,H4,8,H3,8,M6,4,M6,4,M6,4,M6,16,H1,16,H2,16,H3,16,M6,4,M6,4,M6,8,M5,8,M5,8,M6,8,
	M6,4,M6,4,M6,4,M6,16,H1,16,H2,16,H3,16,H6,4,H6,8,M5,8,M5,8,M6,8,M6,4,0U,4,0U,4,0U,4,0U,4,
  {END_SONG,1}

};

song Castle_in_the_Sky[] =//天空之城//1/8拍是8
{
  M6,8,M7,8,
	H1,24,M7,8,H1,16,H3,16,
	M7,16,M7,16,M7,16,M3,8,M3,8,ZERO,8,

  M6,24,M5,8,M6,16,H1,16,
	M5,16,M5,16,M5,16,M3,16,
	M4,24,M3,8,M4,16,H1,16,ZERO,8,

  M3,16,M3,16,ZERO,8,H1,8,H1,8,H1,8,
	M7,24,half_M4,8,M4,16,M7,16,
	M7,16,M7,16,ZERO,16,M6,8,M7,8,ZERO,8,

  H1,24,M7,8,H1,16,H3,16,
	M7,16,M7,16,M7,16,M3,8,M3,8,
	M6,24,M5,8,M6,16,H1,16,ZERO,8,

  M5,16,M5,16,M5,16,M2,8,M3,8,
	M4,16,H1,8,M7,8,M7,8,H1,8,H1,16,
	H2,8,H2,8,H3,8,H1,8,H1,16,H1,16,ZERO,8,

  H1,8,M7,8,M6,8,M6,8,M7,16,half_M5,16,
	M6,16,M6,16,M6,16,H1,8,H2,8,
	H3,24,H2,8,H3,16,H5,16,ZERO,8,

  H2,16,H2,16,H2,16,M5,8,M5,8,
	H1,24,M7,8,H1,16,H3,16,
	H3,16,H3,16,H3,16,H3,16,ZERO,8,

  M6,8,M7,8,H1,16,M7,16,H2,8,H2,8,
	H1,24,M5,8,M5,16,M5,16,
	H4,16,H3,16,H2,16,H1,16,ZERO,8,

  H3,16,H3,16,H3,16,H3,16,
	H6,16,H6,16,H5,16,H5,16,
	H3,8,H2,8,H1,16,H1,16,ZERO,8,H1,8,ZERO,8,

  H2,16,H1,8,H2,8,H2,16,H5,16,
	H3,16,H3,16,H3,16,H3,16,
	H6,16,H6,16,H5,16,H5,16,ZERO,8,

  H3,8,H2,8,H1,16,H1,16,ZERO,8,H1,8,
	H2,16,H1,8,H2,8,H2,16,M7,16,
	M6,16,M6,16,M6,16,M6,8,M7,8,ZERO,8,
  {END_SONG,1}
};

song Little_star[] =//小星星
{
  M1,16,M1,16,M5,16,M5,16,M6,16,M6,16,M5,32,ZERO,8,
	M4,16,M4,16,M3,16,M3,16,M2,16,M2,16,M1,32,ZERO,8,
	M5,16,M5,16,M4,16,M4,16,M3,16,M3,16,M2,32,ZERO,8,
	M5,16,M5,16,M4,16,M4,16,M3,16,M3,16,M2,32,ZERO,8,
	M1,16,M1,16,M5,16,M5,16,M6,16,M6,16,M5,32,ZERO,8,
	M4,16,M4,16,M3,16,M3,16,M2,16,M2,16,M1,32,ZERO,8,
	{END_SONG,1}
};

song INTEL[] =//英特尔经典音乐
{
	H5,16,ZERO,16,M5,32,H1,32,M5,32,H2,32,
	{END_SONG,1}
};

song Song_of_Joy[] =//欢乐颂
{
	{M3,8},{M3,8},{M4,8},{M5,8},
	{M5,8},{M4,8},{M3,8},{M2,8},
	{M1,8},{M1,8},{M2,8},{M3,8},
	{M3,4},{M2,16},{M2,4},ZERO,16,

	{M3,8},{M3,8},{M4,8},{M5,8},
	{M5,8},{M4,8},{M3,8},{M2,8},
	{M1,8},{M1,8},{M2,8},{M3,8},
	{M2,4},{M1,16},{M1,4},ZERO,16,

	{M2,8},{M2,8},{M3,8},{M1,8},
	{M2,8},{M3,16},{M4,16},{M3,8},{M1,8},
	{M2,8},{M3,16},{M4,16},{M3,8},{M2,8},
	{M1,8},{M2,8},{L5,8},ZERO,16,

	{M3,8},{M3,8},{M4,8},{M5,8},
	{M5,8},{M4,8},{M3,8},{M4,16},{M2,16},
	{M1,8},{M1,8},{M2,8},{M3,8},
	{M2,4},{M1,16},{M1,4},ZERO,16,

	{M2,8},{M2,8},{M3,8},{M1,8},
	{M2,8},{M3,16},{M4,16},{M3,8},{M1,8},
	{M2,8},{M3,16},{M4,16},{M3,8},{M2,8},
	{M1,8},{M2,8},{L5,8},ZERO,16,

	{M3,8},{M3,8},{M4,8},{M5,8},
	{M5,8},{M4,8},{M3,8},{M4,16},{M2,16},
	{M1,8},{M1,8},{M2,8},{M3,8},
	{M2,4},{M1,16},{M1,4},ZERO,16,
	{END_SONG,1}
};
static uint8_t Flag=0;	//检测运行状态
static uint8_t pluse=0;//音符节拍 以音节为准 16分音符为最小的 只允许取值1 2 4 8 16(x分音符)
static song* playing=NULL;//当前放的歌曲

static void Music_Set_Tone(uint16_t freq)//设定音符频率
{
	__HAL_TIM_SET_AUTORELOAD(&MTIM,freq);
	__HAL_TIM_SET_COMPARE(&MTIM,MCH,freq/2+1);
	MTIM.Instance->CNT=0;//定时器金句！！！！！！！！！！！！！！！！！！！！！！！！！！
}

void Music_Play(song* head)
{
	if(playing!=NULL)return;//已经有在放的歌曲,不要重复放
	playing=head-1;//-1是因为第一个音符不会被演奏
	HAL_TIM_PWM_Start(&MTIM,MCH);
	Flag=1;
}
void Music_Init(void)
{
	if(playing!=NULL)return;
	Music_Play(jile);
}

void Music_Stop(void) //音乐停止
{
	playing=NULL;
	Flag=0;
	HAL_TIM_PWM_Stop(&MTIM,MCH);
}


#ifdef DANCE  //更骚的跳舞扩展包
static uint8_t danceflag=0x00;
//1代表在正 0代表在负状态
//第[2:0] 由高到低分别为：前后 左右 旋转 云台pitch

static void Dance(void)
{
	if(playing->a==END_SONG)
	{
		Chassis.Run(0,0,0);
		Lift_Target=0;
		Music_Stop();
		return;
	}
	switch(playing->p)
	{
		case 1://最长的音符
			if(danceflag&0x02)
			{
				danceflag&=~0x02;
				Chassis.Run(0,0,-1000);
			}else{
				danceflag|=0x02;
				Chassis.Run(0,0,1000);
			}
			break;
		case 2:
			if(danceflag&0x04)
			{
				danceflag&=~0x04;
				Chassis.Run(-1000,0,0);
			}else{
				danceflag|=0x04;
				Chassis.Run(1000,0,0);
			}
			break;
		case 4:
			if(danceflag&0x08)
			{
				danceflag&=~0x08;
				Chassis.Run(0,-800,0);
			}else{
				danceflag|=0x08;
				Chassis.Run(0,800,0);
			}
			break;
		case 8:
			if(danceflag&0x02)
			{
				danceflag&=~0x02;
				Chassis.Run(0,0,-800);
			}else{
				danceflag|=0x02;
				Chassis.Run(0,0,800);
			}
			break;
		case 16://16分音符
			
			if(danceflag&0x01)
			{
				Lift_Target=10;
				danceflag&=~0x01;
			}else{
				Lift_Target=5;
				danceflag|=0x01;
			}
	}
}
#else
void Dance(void)
{

}
#endif

void Music_Handle(void)//将这个函数放在定时器更新中断里面 建议0.5s为一次周期 可根据歌曲特性改变周期
{
	if(Flag==0)return;
	if(pluse==0)//说明当前音符演奏完成
	{
		playing++;//指向下一个音符
		if(playing->a==END_SONG)//如果播放完毕
		{
			if(IsDance)Dance();
			Music_Stop();
			return;
		}
		
		if(IsDance)Dance();//如果允许跳舞则调用跳舞函数
		//下面三行是为了产生音符之间的中断
		__HAL_TIM_DISABLE(&MTIM);
		Music_Set_Tone(playing->a);//发出相应音符的声音
		osDelay(10);
		__HAL_TIM_ENABLE(&MTIM);
		
		pluse=16/playing->p;
	}else{//说明当前音符时长未到
		pluse--;
	}
}

