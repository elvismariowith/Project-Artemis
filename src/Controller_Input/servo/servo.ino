#include <Servo.h>

Servo myservo;
Servo myservoY;
int default_position = 90;
int default_positionY = 90;
int currentposition = default_position;
int currentpositionY = default_positionY;


int analogPin = A0;

int mapRange(int input){
  return map(input, 0, 677, 0, 180); //Serial.println(charValue)// 1st pair: joystick // 2nd pair: servo 
}
void setup() {
  Serial.begin(9600); // communication with PS4
  myservo.attach(10);
  myservo.write(default_position);

  myservoY.attach(11);
  myservoY.write(default_positionY);
}

void loop() {
  // put your main code here, to run repeatedly:  

  //read joystick
  while (Serial.available()){
    int angle = Serial.parseInt();

    Serial.println(angle);

    if (angle == -1){
      if (currentposition != 0){
        currentposition = currentposition - 2;
        myservo.write(currentposition);
      }
    }

    if (angle == 1){
      if (currentposition != 180){
        currentposition = currentposition + 2;
        myservo.write(currentposition);
      }
      
    }

    if (angle == -2){
      if (currentpositionY != 0){
        currentpositionY = currentpositionY - 2;
        myservoY.write(currentpositionY);
      }
    }

    if (angle == 2){
      if (currentpositionY != 180){
        currentpositionY = currentpositionY + 2;
        myservoY.write(currentpositionY);
      }
      
    }

    //Serial.print("Current position: ");
    Serial.println(angle);
  } 
  
}
