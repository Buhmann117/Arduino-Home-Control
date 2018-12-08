#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Homie.h>
#include <RCSwitch.h>
#include <NewRemoteTransmitter.h>

const int FunkPin = 16;
const int PIN_SOUND =12;
const int PIN_MOTION = 5; 
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
HomieNode funkWZNode("funkWZ", "strip");

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
    temperature = temperature-3.0; // Offset
    humidity = humidity+4.0; // Offset
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

bool funkWZHandler(const HomieRange& range, const String& value) { 
  if (!range.isRange) return false;  // if it's not a range
  if (range.index < 1 || range.index > NUMBER_OF_LED) return false;  // if it's not a valid range
  if (value != "on" && value != "off") return false;  // if the value is not valid
  bool on;
  //Homie.getLogger() << range.index << endl; // for debug
    if (range.index == 1){
      if (on = (value == "on")){       
      //Homie.getLogger() << "ist 2/on" << endl;
        mySwitch.switchOn("10011", "10011");
      }
      else {
      //Homie.getLogger() << "ist 2/off" << endl; 
      mySwitch.switchOff("10011", "10011");
      } 
    }   
    else if (range.index == 2){
      if (on = (value == "on")){
        //Homie.getLogger() << "ist 2/on" << endl;
        mySwitch.switchOff('a', 1, 3);
        mySwitch.switchOn('a', 1, 3);
      }
      else if (on = (value == "off")) {
        //Homie.getLogger() << "ist 2/off" << endl; 
        mySwitch.switchOff('a', 1, 3); 
      }
    }
    else if (range.index == 3){
      if (on = (value == "on")){
        //Homie.getLogger() << "ist 3/on" << endl;
        mySwitch.switchOn("10011", "01101");
      }
      else {
      //Homie.getLogger() << "ist 3/off" << endl; 
      mySwitch.switchOff("10011", "01101");
      } 
    }
    else if (range.index == 4){
      if (on = (value == "on")){
        //Homie.getLogger() << "ist 4/on" << endl;
        mySwitch.switchOn("10011", "00110");
      }
      else {
      //Homie.getLogger() << "ist 4/off" << endl; 
      mySwitch.switchOff("10011", "00110");
      } 
    }
  funkWZNode.setProperty("dev").setRange(range).send(value);  // Update the state of the led
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

  debouncermotion.interval(5000);
  debouncerlight.interval(60);
  debouncersound.interval(5000);

  Homie_setFirmware("Wohnzimmer", "1.0.0");

  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  
  funkWZNode.advertiseRange("dev", 1, NUMBER_OF_LED).settable(funkWZHandler);
  
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
