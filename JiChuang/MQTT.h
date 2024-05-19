#define WIFI_SSID "Wayne的iPhone"       //wifi名
#define WIFI_PASSWD "HBB.20030528" //wifi密码

#define PRODUCT_KEY "k18q7ceN4JU"                        //产品ID
#define DEVICE_NAME "control"                     //设备名
#define DEVICE_SECRET "ee2ea8261ba023d73b98d7458d1e4704" //设备key

//接收消息，订阅主题
#define ALINK_TOPIC_GET "/" PRODUCT_KEY "/" DEVICE_NAME "/user/get"
//设备下发命令的set主题
#define ALINK_TOPIC_PROP_SET "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/service/property/set"
//设备上传数据的post主题
#define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"
//设备post上传数据要用到一个json字符串, 这个是拼接postJson用到的一个字符串
#define ALINK_METHOD_PROP_POST "thing.event.property.post"
//这是post上传数据使用的模板
#define ALINK_BODY_FORMAT "{\"id\":\"%u\",\"version\":\"1.0\",\"method\":\"%s\",\"params\":%s}"

#define LED_B 2 //定义LED灯的引脚


void callback(char *topic, byte *payload, unsigned int length);
void mqttPublish();
void clientReconnect();
void setupWifi();
void WIFI();
void MQTT_setup();
void MQTT();