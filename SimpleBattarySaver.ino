#define CLK 3
#define DIO 2
#define CTRL 9
#define SV A0
#define BTN 4

#include "GyverTM1637.h"
#include "GyverButton.h"
#include <TimerMs.h>

GyverTM1637 disp(CLK, DIO);
GButton btn(BTN, LOW_PULL, NORM_OPEN);
TimerMs tmr(10000, true, true);
TimerMs tmrOff(3000, false, true);
TimerMs tmrMess(200,true);

const float R1 = 20000.0;
const float R2 = 5100.0;
const float VREF = 4.95;
const float ADC_BIT = 1024.0;
const int RATIO = 100;
const int OFF_MIN_VOLT = 1100;
const int OFF_MAX_VOLT = 1110;
const int ON_MIN_VOLT = 1130;

float voltageValue;

void setup() {
  Serial.begin(9600);
  pinMode(CTRL, OUTPUT);
  pinMode(BTN, INPUT);
  ignition();
  disp.clear();
  disp.brightness(7);
  tmrOff.attach(offLine);
  tmrMess.attach(sendDebug);
}

void loop() {
  tmrMess.tick();
  turnDisplay();
  computeState();
}

void sendDebug(){
    Serial.print("V=");
    Serial.println(getVoltage());
}

void offLine() {
  digitalWrite(CTRL, HIGH);
}

void onLine() {
  digitalWrite(CTRL, LOW);
}

float getVoltage() { //Тут надо бы сделать RMS а не среднее арифметическое.
  const int times = 512;
  static unsigned long int val = 0;
  for(int i=0;i<times;i++){
    val += analogRead(SV);
  }
  val /= times;
  float vPin = (val * VREF) / ADC_BIT;
  float vInput = vPin / (R2 / (R1 + R2));
  return vInput;
}

void ignition() {
  offLine();
  int val = getVoltage() * RATIO;
  if (val < OFF_MIN_VOLT) {
    offLine();
  }
  else if (val > OFF_MIN_VOLT) {
    onLine();
  }
}

void computeState() {
  tmrOff.tick();
  int val = getVoltage() * RATIO;
  if (val < OFF_MIN_VOLT) {
    if (!tmrOff.active()) {
      tmrOff.start();
    }
  }
  else if (val >= ON_MIN_VOLT) {
    tmrOff.stop();
    onLine();
  }
}

void displayVolt(int val) {
  int numPos = 3;
  //disp.clear();
  disp.point(POINT_ON);
  while (val) {
    disp.display(numPos, val % 10);
    val = val / 10;
    --numPos;

  }
  if (numPos == 0) {
    disp.display(0, 0x48);
  }
}

void turnDisplay() {
  btn.tick();
  static bool flag = true;
  if (tmr.elapsed()) flag = false;
  if (btn.isSingle()) {
    flag = true;
    tmr.start();
  }
  if (flag) {
    displayVolt(getVoltage() * RATIO);
  }
  else {
    disp.clear();
    disp.point(POINT_OFF);
  }
}
