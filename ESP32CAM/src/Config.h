#pragma once

#include <ArduinoWebsockets.h>

#define SSID1 "leisure"
#define PWD1 "shadow27"

const int FPS = 13;                                                     // 每秒传输帧数
const int interval = 100;                                               // We will handle web client requests every 50 ms (20 Hz)
QueueHandle_t streamingClients = xQueueCreate(5, sizeof(WiFiClient *)); // 可同时连接的客户端个数2

// 局域网图传 http://192.168.212.126/mjpeg/1

/*Websocket配置*/
const char *websockets_server_host = "192.168.212.153";
const uint16_t websockets_server_port = 8888;
using namespace websockets;
WebsocketsClient client1;

/*函数声明*/
void mjpegCB(void *pvParameters);
void camCB(void *pvParameters);
char *allocateMemory(char *aPtr, size_t aSize);
void handleJPGSstream(void);
void streamCB(void *pvParameters);
void handleNotFound();
void CamConfig();
void ConnectWIFI();