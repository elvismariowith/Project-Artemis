#include <Servo.h>

Servo myservo;
int default_position = 90;
int currentposition = default_position;

int analogPin = A0;

int mapRange(int input){
  return map(input, 0, 677, 0, 180); //Serial.println(charValue)// 1st pair: joystick // 2nd pair: servo 
}
void setup() {
  Serial.begin(9600); // communication with PS4
  myservo.attach(10);
  myservo.write(default_position);
}

void loop() {
  // put your main code here, to run repeatedly:  

  //read joystick
  if (Serial.available() > 0){
    int angle = Serial.parseInt();

    if (angle >= 0 && angle <= 180) {
      myservo.write(angle);
      Serial.print("Moving servo to: ");
      Serial.println(angle);
    }
  } 
  
}
