#include "esp32-hal-gpio.h"
#include "shuibong.h"

void ShuiBong_init(void){
  pinMode(ShuiBong_PIN1, OUTPUT);
  pinMode(ShuiBong_PIN2, OUTPUT);
  pinMode(ShuiBong_PIN3, OUTPUT);
  digitalWrite(ShuiBong_PIN1, HIGH);
  digitalWrite(ShuiBong_PIN2, HIGH);
  digitalWrite(ShuiBong_PIN3, HIGH);
}

void OpenMV_init(void){
  pinMode(OpenMV_PIN1, INPUT_PULLDOWN);
  pinMode(OpenMV_PIN2, INPUT_PULLDOWN);
  pinMode(OpenMV_PIN3,OUTPUT);
  digitalWrite(OpenMV_PIN1, LOW);
  digitalWrite(OpenMV_PIN2, LOW);
  digitalWrite(OpenMV_PIN2, HIGH);
}

void ziou_init(void){
  pinMode(ziou_PIN, INPUT);
  pinMode(noziou_PIN, INPUT);
  digitalWrite(ziou_PIN, LOW);
  digitalWrite(noziou_PIN, LOW);
}

void ShuiBong_pengtou(int x){
  ShuiBong1(x);
  ShuiBong2(x);
}

void ShuiBong_qinxi(int x){
  ShuiBong3(x);
}

void ziou(){
  pinMode(ziou_PIN, OUTPUT);
  digitalWrite(ziou_PIN,1);
}       
