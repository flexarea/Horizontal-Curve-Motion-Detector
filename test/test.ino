#include <Arduino.h>

#define RCWL 26

void setup(){
  Serial.begin(9600);
  pinMode(RCWL, INPUT);
  //pinMode(RCWL, INPUT_PULLDOWN);
  pinMode(LED_BUILTIN, OUTPUT);

}

void loop(){
  int pinState = digitalRead(RCWL);
  if(pinState == HIGH){
    digitalWrite(LED_BUILTIN, HIGH);
  }else{
    digitalWrite(LED_BUILTIN, LOW);
  }
  //delay(1000);
}
