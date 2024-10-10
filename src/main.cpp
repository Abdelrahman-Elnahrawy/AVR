#include <Arduino.h>
#include "avr8-stub.h"
#define LED PIN_PB7
#define BUTTON PIN_PE0


void setup() {
  pinMode(LED,OUTPUT);
  pinMode(BUTTON,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(BUTTON)==0){
  digitalWrite(LED,HIGH);
  delay(100);
  }
  else{
  digitalWrite(LED,LOW);
    delay(100);
  }
  
}

