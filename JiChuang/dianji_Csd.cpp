#include "esp32-hal-gpio.h"
#include "dianji_Csd.h"

void dianji_init(void){
    pinMode(dianji1, INPUT);
    pinMode(dianji2, INPUT);
    pinMode(dianji3, INPUT);
    pinMode(dianji4, INPUT);
    // digitalWrite(dianji1, LOW);
    // digitalWrite(dianji2, LOW);
    // digitalWrite(dianji3, LOW);
    // digitalWrite(dianji4, LOW);

}

void Csd_init(void){
  pinMode(Csd_PIN, OUTPUT);
  pinMode(shuipingCsd_PIN, OUTPUT);
  digitalWrite(Csd_PIN, HIGH);
  digitalWrite(shuipingCsd_PIN, LOW);
}

void Output_1(uint8_t pin){
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}

void Stop(){
  pinMode(dianji1,INPUT);
  pinMode(dianji2,INPUT);
  pinMode(dianji3,INPUT);
  pinMode(dianji4,INPUT);
  // pinMode(STOP, OUTPUT);
  // digitalWrite(STOP, HIGH);
}
