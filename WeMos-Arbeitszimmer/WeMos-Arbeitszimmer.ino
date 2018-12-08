#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Homie.h>
#include <RCSwitch.h>
#include <NewRemoteTransmitter.h>

const int FunkPin = 16;
const int PIN_SOUND =5;
const int PIN_MOTION = 12; //12
const int PIN_LIGHT = 4;  // 4
const int PIN_TEMP=14;   // 14
int lastSensorValue=-1;
int lastLightValue = -1;  
int lastSoundValue=-1; 
const int TEMPERATURE_INTERVAL = 120; // time [sec] between measurements 
unsigned long lastTemperatureSent = 0;
const unsigned char NUMBER_OF_LED = 10;
float humidity, temperature;

#define DHTTYPE DHT22                                      
DHT dht(PIN_TEMP, DHTTYPE);

Bounce debouncersound = Bounce(); // Bounce is built into Homie, so you can use it without including it first
Bounce debouncerlight = Bounce();
Bounce debouncermotion = Bounce();

RCSwitch mySwitch = RCSwitch();

HomieNode soundNode("sound", "door");
HomieNode sensorNode("motion", "door");
HomieNode lightNode("light", "door");
HomieNode temperatureNode("temperature", "temperature");
HomieNode humidityNode("humidity", "temperature");
HomieNode funkANode("funkA", "strip");
HomieNode funkGNode("funkG", "strip");
HomieNode funkFNode("funkF", "strip");


void setupHandler() {
  temperatureNode.setProperty("unit").send("c");
  humidityNode.setProperty("unit").send("%");
}

void loopHandler() {
  int soundValue = debouncersound.read();
  int sensorValue = debouncermotion.read();
  int lightValue = debouncerlight.read();
  //Homie.getLogger() << (sensorValue) << endl;  
  if (sensorValue != lastSensorValue) {
     if (sensorValue == 1){
      Homie.getLogger() << "Sensor is High" << endl;
      sensorNode.setProperty("state").send("on");
     }
     else if (sensorValue == 0){
      Homie.getLogger() << "Sensor is Low" << endl;
      sensorNode.setProperty("state").send("off");
     }
     //Homie.getLogger() << "Light is now " << (lightValue ? "on" : "off") << endl;
     lastSensorValue = sensorValue;
  }
  if (lightValue != lastLightValue) {
     if (lightValue == 0){
      Homie.getLogger() << "Light is on" << endl;
      lightNode.setProperty("state").send("on");
     }
     else if (lightValue == 1){
      Homie.getLogger() << "Light is off" << endl;
      lightNode.setProperty("state").send("off");
     } 
     //Homie.getLogger() << "Light is now " << (lightValue ? "on" : "off") << endl;
     //lightNode.setProperty("open").send(lightValue ? "true" : "false");
     lastLightValue = lightValue;
  }
  //Homie.getLogger() << (sensorValue) << endl;  // light on: 0 light off: 1
  if (soundValue != lastSoundValue) {
     if (soundValue == 1){
      Homie.getLogger() << "Sound is High" << endl;
      soundNode.setProperty("state").send("on");
     }
     else if (soundValue == 0){
      Homie.getLogger() << "Sound is Low" << endl;
      soundNode.setProperty("state").send("off");
     }
     //Homie.getLogger() << "Light is now " << (lightValue ? "on" : "off") << endl;
     lastSoundValue = soundValue;
  }
  if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL * 1000UL || lastTemperatureSent == 0) {
    //float temperature = 22; // Fake temperature here, for the example
    humidity = dht.readHumidity();                           
    temperature = dht.readTemperature();
    temperature = temperature-4.6; // Offset
    if (isnan(temperature) || isnan(humidity)){
      Serial.println("DHT22 konnte nicht ausgelesen werden");       
    }
    else {
      Homie.getLogger() << "Temperature: " << temperature << " °C" << endl;
      Homie.getLogger() << "Humidity: " << humidity << " %" << endl;
      temperatureNode.setProperty("degrees").send(String(temperature));
      humidityNode.setProperty("degrees").send(String(humidity));
    }
    lastTemperatureSent = millis();
  }
}

bool funkAHandler(const HomieRange& range, const String& value) {
  if (!range.isRange) return false;  // if it's not a range
  if (range.index < 1 || range.index > NUMBER_OF_LED) return false;  // if it's not a valid range
  if (value != "on" && value != "off") return false;  // if the value is not valid
  bool on;
  if (range.index == 1){
    NewRemoteTransmitter transmitter(38153216, FunkPin, 266, 2);
    if (on = (value == "on")){
      //Homie.getLogger() << "ist 3/on" << endl;
      transmitter.sendDim(5, 15);// last No is repeating the transmitted code 2^(2)=8 times
    }
    else {
      //Homie.getLogger() << "ist 3/off" << endl; 
      transmitter.sendDim(5, 0); 
    }
  }
  else if (range.index == 2){
      NewRemoteTransmitter transmitter(38153216, FunkPin, 266, 3); // last No is repeating the transmitted code
      if (on = (value == "on")){
        //Homie.getLogger() << "ist 1/on" << endl;
        transmitter.sendDim(6, 15);
      }
      else if (on = (value == "off")) {
        //Homie.getLogger() << "ist 1/off" << endl; 
        transmitter.sendDim(6, 0); 
      }
    }
   else if (range.index == 3){
      mySwitch.setProtocol(4);
      mySwitch.setRepeatTransmit(10);
      if (on = (value == "on")){
        //Homie.getLogger() << "ist 1/on" << endl;
        mySwitch.send(28720, 24);
      }
      else if (on = (value == "off")) {
        //Homie.getLogger() << "ist 1/off" << endl; 
        mySwitch.send(807328, 24);
      }
    }
  funkANode.setProperty("dev").setRange(range).send(value);  // Update the state of the led
  Homie.getLogger() << "Device " << range.index << " is " << value << endl;

  return true;
}

bool funkFHandler(const HomieRange& range, const String& value) {
  if (!range.isRange) return false;  // if it's not a range
  if (range.index < 1 || range.index > NUMBER_OF_LED) return false;  // if it's not a valid range
  if (value != "on" && value != "off") return false;  // if the value is not valid

  bool on;
  if (range.index == 1){ // 
    if (on = (value == "on")){
      //Homie.getLogger() << "ist 5/on" << endl;
      mySwitch.switchOn('a', 1, 1);
     }
    else {
      //Homie.getLogger() << "ist 5/off" << endl; 
      mySwitch.switchOff('a', 1, 1); 
    }
  }
  funkFNode.setProperty("dev").setRange(range).send(value);  // Update the state of the led
  Homie.getLogger() << "Device " << range.index << " is " << value << endl;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  mySwitch.enableTransmit(FunkPin);
  dht.begin();  
  
  pinMode(PIN_SOUND, INPUT);
  pinMode(PIN_MOTION, INPUT);
  pinMode(PIN_LIGHT, INPUT);
  debouncermotion.attach(PIN_MOTION);
  debouncerlight.attach(PIN_LIGHT);
  debouncersound.attach(PIN_SOUND);

  debouncermotion.interval(10);
  debouncerlight.interval(60);
  debouncersound.interval(1);

  Homie_setFirmware("awesome-Arbeitszimmer", "1.0.0");

  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  
  funkANode.advertiseRange("dev", 1, NUMBER_OF_LED).settable(funkAHandler);
  //funkCNode.advertiseRange("dev", 1, NUMBER_OF_LED).settable(funkCHandler);
  //funkGNode.advertiseRange("dev", 1, NUMBER_OF_LED).settable(funkGHandler);
  //funkFNode.advertiseRange("dev", 1, NUMBER_OF_LED).settable(funkFHandler);
  //funkPNode.advertiseRange("dev", 1, NUMBER_OF_LED).settable(funkPHandler);
  
  temperatureNode.advertise("unit");
  temperatureNode.advertise("degrees");
  humidityNode.advertise("unit");
  humidityNode.advertise("degrees");
  
  soundNode.advertise("state");
  sensorNode.advertise("state");   
  lightNode.advertise("state");

  //Homie.reset();
  //Homie.disableLedFeedback(); // Disable Onboard LED to use TX
  Homie.disableResetTrigger(); // disable GPIO 0 as a reset
  Homie.setup();
}

void loop() {
  Homie.loop();
  debouncersound.update();
  debouncermotion.update();
  debouncerlight.update();
}
