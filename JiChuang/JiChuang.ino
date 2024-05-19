/*
17-水平传送带：TXT信号控制
18-倾斜传送带：继电器控制




*/


#include "shuibong.h"
#include "Bluetooth.h"
#include "dianji_Csd.h"
#include "MQTT.h"





void setup()
{
  Bluetooth_init();
  dianji_init();
  ShuiBong_init();
  OpenMV_init();
  ziou_init();
  Csd_init();
  // Csd(1);
  /**************MQTT-setup***************/
  MQTT_setup();
  /*************MQTT-setup****************/
}
 
void loop()
{
  //wifi连接
  WIFI();
  //mqtt客户端监听
  MQTT();
  // pinMode(OpenMV_PIN1, INPUT_PULLDOWN);
  // Bluetooth();
  //水泵运行
  ShuiBong_pengtou(1);
  ShuiBong_qinxi(1);
  //两个传送带运行
  // shuipingCsd(0);
  // if(OpenMV_sig1){   //一旦检测到有藕
  //   Serial.print("识别到有藕");
  //   Stop();                       //机器停止前进
  //   ShuiBong_pengtou(0);          //最前端喷头停止喷水
  //   //Csd(0);                       //倾斜传送带停止（此时最前端的引流模块停止与否？
  //   ShuiBong_qinxi(1);            //清洗一秒
  //   delay(1000);
  //   shuipingCsd(1);               //发出信号，让水平传送带开始运动
  //   delay(200);
  //               //检测到有子藕(有藕)
  //   ziou();                    //子藕引脚发出信号到TXT控制板
  //   Serial.print("sig1\n");
  //   delay(10000);                 //延时一段时间，防止信号识别不到
    
    // else{
    //   noziou(1);                  //同理
    //   Serial.print("sig2\n");
    //   delay(200);
    // }
  // }
  //   //给OpenMV发送信号，使其继续识别
  // OpenMV_init();
    //检测有没有断线

}
