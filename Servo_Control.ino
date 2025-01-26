#include <Servo.h>
#include <stdio.h>


Servo myservo;  // create servo object to control a servo
int potpin = 0;  // analog pin used to connect the potentiometer
int val;    // variable to read the value from the analog pin
int rando = 0;
void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
}

void loop() {
  val = Serial.parseInt();
  delay(1000);
  myservo.write(val);                  // sets the servo position according to the scaled value
  Serial.println(val);
                             // waits for the servo to get there
}