#include <Adafruit_Sensor.h>
#include <Homie.h>
#include <Servo.h>

const int Servo1Pin = 0; // Pin for motor Start
const int Servo2Pin = 2; // Pin for motor Mahlen aus
const unsigned char NUMBER_OF_LED = 10;

int servoAngle = 0;   // servo position in degrees
Servo servo1;  
Servo servo2;

HomieNode motorNode("motor", "strip");

bool motorHandler(const HomieRange& range, const String& value) {
  if (!range.isRange) return false;  // if it's not a range
  if (range.index < 1 || range.index > NUMBER_OF_LED) return false;  // if it's not a valid range
  if (value != "on" && value != "off") return false;  // if the value is not valid
  bool on;
  if (range.index == 1){
    if (on = (value == "on")){
      Homie.getLogger() << "go to 40°" << endl;
      servo1.write(40);      // Turn SG90 servo back to 90 degrees (center position)
    }
    else {
      Homie.getLogger() << "go to 35°" << endl;  
      servo1.write(35);      // Turn SG90 servo Left to 45 degrees
    }
  }
  else if (range.index == 2){
      if (on = (value == "on")){
        Homie.getLogger() << "go to 48°" << endl;
        servo2.write(48);      // Turn SG90 servo back to 90 degrees (center position)
      }
      else {
        Homie.getLogger() << "go to 35°" << endl;  
        servo2.write(35);      // Turn SG90 servo Left to 45 degrees
      }
    }
  motorNode.setProperty("dev").setRange(range).send(value);  // Update the state of the led
  Homie.getLogger() << "Device " << range.index << " is " << value << endl;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  servo1.attach(Servo1Pin);
  servo2.attach(Servo2Pin);

  Homie_setFirmware("Coffee Machine", "1.0.0");
  motorNode.advertiseRange("dev", 1, NUMBER_OF_LED).settable(motorHandler);

  //Homie.reset();
  //Homie.disableLedFeedback(); // Disable Onboard LED to use TX
  Homie.disableResetTrigger(); // disable GPIO 0 as a reset
  Homie.setup();
}

void loop() {
  Homie.loop();
}
