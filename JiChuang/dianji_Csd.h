#define dianji1 4   //前进
#define dianji2 5   //后退
#define dianji3 6   //左转弯
#define dianji4 7   //右转弯

#define Csd_PIN 18
#define shuipingCsd_PIN 17
#define Csd(x)        digitalWrite(Csd_PIN,x)    //倾斜传送带运行与否
#define Csd_sig       digitalRead(Csd_PIN)
#define shuipingCsd(x)        digitalWrite(shuipingCsd_PIN,x)   //水平传送带运行与否
#define shuipingCsd_sig       digitalRead(shuipingCsd_PIN)

void Csd_init(void);
void dianji_init(void);
void Output_1(uint8_t pin);
void Stop();