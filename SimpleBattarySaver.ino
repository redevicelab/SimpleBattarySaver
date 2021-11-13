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

const float R1 = 30000.0;
const float R2 = 10000.0;
const float VREF = 5.0;
const float ADC_BIT = 1024.0;
const int OFF_MIN_VOLT = 1100;
const int OFF_MAX_VOLT = 1110;
const int ON_MIN_VOLT = 1130;

int val;

void setup() {
  Serial.begin(9600);
  pinMode(CTRL, OUTPUT);
  pinMode(BTN, INPUT);
  ignition();
  disp.clear();
  disp.brightness(7);
  tmrOff.attach(offLine);
}

void loop() {
  tmrOff.tick();
  static bool flag = true;
  if (tmr.elapsed()) flag = false;
  btn.tick();
  if (btn.isSingle()) {
    flag = true;
    tmr.start();
  }
  if (flag) {
    displayVolt(getVoltage() * 100);
  }
  else {
    disp.clear();
    disp.point(POINT_OFF);
  }
  computeState();
}

float getVoltage() {
  int val = analogRead(SV);
  float vPin = (val * VREF) / ADC_BIT;
  float vInput = vPin / (R2 / (R1 + R2));
  return vInput;
}

void offLine() {
  digitalWrite(CTRL, HIGH);
}

void onLine() {
  digitalWrite(CTRL, LOW);
}

void ignition() {
  offLine();
  int val = getVoltage() * 100;
  if (val < OFF_MIN_VOLT) {
    offLine();
  }
  else if (val > OFF_MIN_VOLT) {
    onLine();
  }
}

void computeState() {
  int val = getVoltage() * 100;
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
  int pos = 3;
  disp.clear();
  disp.point(POINT_ON);
  while (val) {
    disp.display(pos, val % 10);
    val = val / 10;
    pos--;
  }
}
