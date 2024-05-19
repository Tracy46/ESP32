#include "esp32-hal-gpio.h"
#include "dianji_Csd.h"
#include "Bluetooth.h"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

uint8_t txValue = 0;                         //后面需要发送的值
BLEServer *pServer = NULL;                   //BLEServer指针 pServer
BLECharacteristic *pTxCharacteristic = NULL; //BLECharacteristic指针 pTxCharacteristic
bool deviceConnected = false;                //本次连接状态
bool oldDeviceConnected = false;             //上次连接状态d

 
class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };
 
    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};
 
class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue(); //接收信息
 
        if (rxValue.length() > 0)
        { //向串口输出收到的值
            Serial.print("RX: ");
            for (int i = 0; i < rxValue.length(); i++)
                Serial.print(rxValue[i]);
            Serial.println();
            if (rxValue[0]=='a')        //前进
              Output_1(dianji1);
            else if (rxValue[0]=='b')
              Stop();    //STOP
            else if (rxValue[0]=='c')   //后退
              Output_1(dianji2);
            else if (rxValue[0]=='d')   //左转弯
              Output_1(dianji3); 
            else if (rxValue[0]=='e')
              Output_1(dianji4);              
        }
    }
};

void Bluetooth_init(void){
    Serial.begin(115200);
 
    // 创建一个 BLE 设备
    BLEDevice::init("ESP32BLE");//在这里面是ble的名称
 
    // 创建一个 BLE 服务
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks()); //设置回调
    BLEService *pService = pServer->createService(SERVICE_UUID);
 
    // 创建一个 BLE 特征
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks()); //设置回调
 
    pService->start();                  // 开始服务
    pServer->getAdvertising()->start(); // 开始广播
    Serial.println(" 等待一个客户端连接，且发送通知... ");
}

void Bluetooth(void){
      // deviceConnected 已连接
    if (deviceConnected)
    {
        pTxCharacteristic->setValue(&txValue, 1); // 设置要发送的值为1
        pTxCharacteristic->notify();              // 广播
        txValue++;                                // 指针数值自加1
        delay(2000);                              // 如果有太多包要发送，蓝牙会堵塞
    }
 
    // disconnecting  断开连接
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500);                  // 留时间给蓝牙缓冲
        pServer->startAdvertising(); // 重新广播
        Serial.println(" 开始广播 ");
        oldDeviceConnected = deviceConnected;
    }
 
    // connecting  正在连接
    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}