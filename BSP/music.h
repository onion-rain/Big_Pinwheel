#ifndef MUSIC_WMD
#define MUSIC_WMD

typedef struct
{
	unsigned short int a; //代表音符
	unsigned char p;      //代表音符节拍
}song;
//歌曲结构体 记得加结束符 格式为{对应音，该音是什么音符}

void Music_Handle(void);//将这个函数放在定时器更新中断里面 建议0.5s为一次周期 可根据歌曲特性改变周期
void Music_Play(song* head);
void Music_Init(void);
void Music_Stop(void); //音乐停止

extern song warning[];
extern song warning1[];
extern song warning2[];
extern song warning3[];
extern song warning4[];
extern song warning5[];
extern song warning6[];
extern song warning7[];
extern song warning8[];
extern song jile[];
extern song Castle_in_the_Sky[];
extern song ItsOK[];
extern song Little_star[];
extern song INTEL[];
extern song Song_of_Joy[];

extern unsigned char IsDance;

#define BEEP_ON __HAL_TIM_SET_AUTORELOAD(&MTIM,1000);\
                __HAL_TIM_SET_COMPARE(&MTIM,MCH,501);\
                HAL_TIM_PWM_Start(&MTIM,MCH);
#define BEEP_OFF HAL_TIM_PWM_Stop(&MTIM,MCH);
#endif
