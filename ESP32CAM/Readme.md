### 🍕ESP32CAM引脚分布
* 闪光灯引脚：IO4
* IO1:TX
* IO3:RX
可能会遇到的问题：
1. 图像的旋转角度可以进行调整
2. TF卡最大32G，支持FAT32格式
### 🍔LED测试
```C++
#include <Arduino.h>

void setup()
{
  pinMode(4, OUTPUT);
}

void loop()
{
  digitalWrite(4, HIGH);
  delay(1000);
  digitalWrite(4, LOW);
  delay(1000);
}
```
### 🍄SD卡操作
嵌入式设备中常用的存储设备：flash、Eeprom、SD卡、U盘。SD卡的好处是容量大，读写速度相对较快(可以使用SDIO或SPI接口通信)。SDI接线连接：
| ESP32           | MicroSD | 说明                           |
| --------------- | ------- | ------------------------------ |
| GPIO18\SPI_CLK  | CLK     | SPI时钟线                      |
| GPIO23\SPI_MOSI | MOSI    | SPI数据线，ESP32输出，SD卡输入 |
| GPIO19\SPI_MISO | MISO    | SPI数据线，ESP32输入，SD卡输出 |
| GPIO5\SPI_CS    | CS      | SPI片选                        |
#### SD库API介绍
| 函数       | 说明                                                                          | 示例                       |
| ---------- | ----------------------------------------------------------------------------- | -------------------------- |
| begin()    | 初始化函数，可以根据实际使用入参（片选引脚，SPI引脚等），不入参的话按默认配置 | SD.begin(); SD.begin(4);   |
| cardType() | 读取SD卡类型                                                                  | card_type = SD.cardType(); |
| open()     | 打开指定路径或文件                                                            | fs.open(“/test.txt”);      |
| remove()   | 删除文件                                                                      | fs.remove(“/test.txt”);    |
| write()    | 往文件里写数据                                                                | file.write(“hello world”); |
| read()     | 从文件里读数据                                                                | size = file.read(buf, 10); |
| mkdir()    | 创建目录                                                                      | fs.mkdir(“/”);             |
```c++
// 需要添加的头文件
#include "FS.h"
#include "SD.h"
#include "SPI.h"
```