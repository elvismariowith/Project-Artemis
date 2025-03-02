#include <Servo.h>

Servo myservo;
int default_position = 90;
int currentposition = default_position; 

int analogPin = 10;

int mapRange(int input){
  return map(input, 0, 677, 0, 180); //Serial.println(charValue)// 1st pair: joystick // 2nd pair: servo 
}
void setup() {
  Serial.end(); // close any previous serial connections
  Serial.begin(9600); // communication with PS4
  myservo.attach(analogPin);
  myservo.write(default_position);
}

void loop() {
  // put your main code here, to run repeatedly:  

  //read joystick
  if (Serial.available() > 0){
    int angle = Serial.parseInt();
    
    if (currentposition >= 0 && currentposition <= 180) {
      currentposition = currentposition + angle;
      myservo.write(currentposition);
      if (currentposition < 0){
        Serial.println("Max boundary for left!");
        currentposition = 0;
      }
      if (currentposition > 180){
        Serial.println("Max boundary for right!");
        currentposition = 180;
      }
     
      Serial.print("Moving servo to: ");
      Serial.println(angle);
      Serial.print("Current position of server: ");
      Serial.println(currentposition);
    }
  } 
  
}
