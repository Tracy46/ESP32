#include <Arduino.h>
#include <ArduinoJson.h>
#include <aliyun_mqtt.h>
#include "PubSubClient.h"
#include "WiFi.h"
#include "Ticker.h"
#include "MQTT.h"
#include "esp32-hal-gpio.h"
#include "dianji_Csd.h"

int postMsgId = 0; //记录已经post了多少条
Ticker tim1;       //这个定时器是为了每5秒上传一次数据

/*------------------------------------------------------------------------------------------*/

WiFiClient espClient;               //创建网络连接客户端
PubSubClient mqttClient(espClient); //通过网络客户端连接创建mqtt连接客户端

//连接WIFI相关函数
void setupWifi()
{
  delay(10);
  Serial.println("连接WIFI");
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  while (!WiFi.isConnected())
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("OK");
  Serial.println("Wifi连接成功");
}

//重连函数, 如果客户端断线,可以通过此函数重连
void clientReconnect()
{
  while (!mqttClient.connected()) //再重连客户端
  {
    Serial.println("reconnect MQTT...");
    if (connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.println("failed");
      Serial.println(mqttClient.state());
      Serial.println("try again in 5 sec");
      delay(5000);
    }
  }
}
//mqtt发布post消息(上传数据)
void mqttPublish()
{
  if (mqttClient.connected())
  {
    //先拼接出json字符串
    char param[32];
    char jsonBuf[128];
    sprintf(param, "{\"control\":%d}", digitalRead(LED_B)); //我们把要上传的数据写在param里
    postMsgId += 1;
    sprintf(jsonBuf, ALINK_BODY_FORMAT, postMsgId, ALINK_METHOD_PROP_POST, param);
    //再从mqtt客户端中发布post消息
    if (mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf))
    {
      Serial.print("Post message to cloud: ");
      Serial.println(jsonBuf);
    }
    else
    {
      Serial.println("Publish message to cloud failed!");
    }
  }
}
//收到set主题的命令下发时的回调函数,(接收命令)
void callback(char *topic, byte *payload, unsigned int length)
{
  if (strstr(topic, ALINK_TOPIC_GET))
  //如果收到的主题里包含字符串ALINK_TOPIC_PROP_SET(也就是收到/sys/a17lGhkKwXs/esp32LightHome/thing/service/property/set主题)
  {
    Serial.println("收到下发的命令主题:");
    Serial.println(topic);
    Serial.println("下发的内容是:");
    payload[length] = '\0'; //为payload添加一个结束附,防止Serial.println()读过了
    Serial.println((char *)payload);

    //接下来是收到的json字符串的解析
    DynamicJsonDocument doc(100);
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.println("parse json failed");
      return;
    }
    JsonObject setAlinkMsgObj = doc.as<JsonObject>();
    serializeJsonPretty(setAlinkMsgObj, Serial);
    Serial.println();

    //这里是一个控制逻辑
    int Control = setAlinkMsgObj["params"]["control"];
    if(Control == 1){                       //前进
      Serial.println("go ok!");
      Output_1(dianji1);
      mqttPublish(); //由于将来做应用可能要获取灯的状态,所以在这里发布一下
    }
    else if(Control == 4){                 //后退
      Serial.println("back ok!");
      Output_1(dianji2);
      mqttPublish(); 
    }
    else if(Control == 3){                 //右转
      Serial.println("right ok!");
      Output_1(dianji4);
      mqttPublish(); 
    }
    else if(Control == 2){                 //左转
      Serial.println("left ok!");
      Output_1(dianji3);
      mqttPublish(); 
    }
    else if(Control == 0){                 //停止
      Serial.println("stop ok!");
      Stop(); 
      mqttPublish(); 
    }         
  }
}

void WIFI()
{
  if (!WiFi.isConnected()) //先看WIFI是否还在连接
  {
    setupWifi();
  }
  else //如果WIFI连接了,
  {
    if (!mqttClient.connected()) //再看mqtt连接了没
    {
      Serial.println("mqtt disconnected!Try reconnect now...");
      Serial.println(mqttClient.state());
      clientReconnect();
    }
  }
}

void MQTT_setup(){
  // pinMode(LED_B, OUTPUT);
  Serial.begin(115200);
  setupWifi();
  if (connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET))
  {
    Serial.println("MQTT服务器连接成功!");
  };
  //订阅topic
  mqttClient.subscribe(ALINK_TOPIC_GET);
  mqttClient.setCallback(callback); //绑定收到set主题时的回调(命令下发回调)
  //tim1.attach(5, mqttPublish);      //启动每5秒发布一次消息
}

void MQTT()
{
  mqttClient.loop();
}

// void setup()
// {
//   pinMode(LED_B, OUTPUT);
//   Serial.begin(115200);
//   setupWifi();
//   if (connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET))
//   {
//     Serial.println("MQTT服务器连接成功!");
//   };
//   mqttClient.setCallback(callback); //绑定收到set主题时的回调(命令下发回调)
//   tim1.attach(5, mqttPublish);      //启动每5秒发布一次消息
// }

// void loop()
// {
//   //检测有没有断线
//   if (!WiFi.isConnected()) //先看WIFI是否还在连接
//   {
//     setupWifi();
//   }
//   else //如果WIFI连接了,
//   {
//     if (!mqttClient.connected()) //再看mqtt连接了没
//     {
//       Serial.println("mqtt disconnected!Try reconnect now...");
//       Serial.println(mqttClient.state());
//       clientReconnect();
//     }
//   }

//   //mqtt客户端监听
//   mqttClient.loop();
// }