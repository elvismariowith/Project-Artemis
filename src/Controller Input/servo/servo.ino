#include <Servo.h>

Servo myservo;
int default_position = 90;
int currentposition = default_position;
int digitalPin = 10;



void setup() {
  Serial.end();
  Serial.begin(9600); // communication with PS4
  myservo.attach(digitalPin);
  myservo.write(default_position);
}

void loop() {
  // Read joystick input
  while (Serial.available()) {
    int direction = Serial.parseInt();

    // Print raw input
    // Serial.println(direction); //<- sending data to the serial port
    delay(10); // small delay for serial stability

    if (direction == -1) {
      if (currentposition > 0) {
        currentposition -= 5;
        myservo.write(currentposition);
      }
    }

    if (direction == 1) {
      if (currentposition < 180) {
        currentposition += 5;
        myservo.write(currentposition);
      }
    }

    //Serial.print("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    //Serial.println(currentposition);
  }


}
