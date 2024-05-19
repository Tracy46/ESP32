#define ShuiBong_PIN1 9
#define ShuiBong_PIN2 10
#define ShuiBong_PIN3 14

#define ShuiBong1(x)        digitalWrite(ShuiBong_PIN1,x)
#define ShuiBong2(x)        digitalWrite(ShuiBong_PIN2,x)
#define ShuiBong3(x)        digitalWrite(ShuiBong_PIN3,x)

#define OpenMV_PIN1 39       
#define OpenMV_PIN2 38
#define OpenMV_PIN3 36         //给OpenMV发送信号启动

#define OpenMV_sig1    digitalRead(OpenMV_PIN1)    //有子藕
#define OpenMV_sig2    digitalRead(OpenMV_PIN2)    //无子藕
#define OpenMV_strat(x)   digitalWrite(OpenMV_PIN3, x)

#define ziou_PIN   45
#define noziou_PIN 48

#define noziou(x)      digitalWrite(noziou_PIN,x)


void ziou();
void ShuiBong_init(void);
void OpenMV_init(void);
void ziou_init(void);
void ShuiBong_pengtou(int x);
void ShuiBong_qinxi(int x);