#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Homie.h>
#include <RCSwitch.h>
#include <NewRemoteTransmitter.h>

const int FunkPin = 16; // 16
const int PIN_SOUND =4;
const int PIN_MOTION1 = 12; 
const int PIN_MOTION2 = 13; 
const int PIN_LIGHT = 5; //5
const int PIN_TEMP= 14; //  14
int lastSensorValue1=-1;
int lastSensorValue2=-1;
int lastLightValue = -1;  
int lastSoundValue=-1;
bool off; // ist true wenn das licht aus ist NOETIG damit licht nicht flackert!
const int TEMPERATURE_INTERVAL = 120; // time [sec] between measurements 
unsigned long lastTemperatureSent = 0;
const unsigned char NUMBER_OF_LED = 10;
float humidity, temperature;

#define DHTTYPE DHT22                                      
DHT dht(PIN_TEMP, DHTTYPE);

Bounce debouncersound = Bounce(); // Bounce is built into Homie, so you can use it without including it first
Bounce debouncerlight = Bounce();
Bounce debouncermotion1 = Bounce();
Bounce debouncermotion2 = Bounce();

RCSwitch mySwitch = RCSwitch();

HomieNode soundNode("sound", "door");
HomieNode sensorNode1("motion1", "door");
HomieNode sensorNode2("motion2", "door");
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
  int sensorValue1 = debouncermotion1.read();
  int sensorValue2 = debouncermotion2.read();
  int lightValue = debouncerlight.read();
  //Homie.getLogger() << (sensorValue) << endl;  
  if ((sensorValue1 != lastSensorValue1)) {  // && (off == true)
     if (sensorValue1 == 1){
      Homie.getLogger() << "Sensor1 is High" << endl;
      //mySwitch.switchOff('a', 1, 1); 
      //mySwitch.switchOn('a', 1, 1);
      sensorNode1.setProperty("state").send("on");
      //funkFNode.setProperty("dev").setRange(1).send("on");
     }
     else if (sensorValue1 == 0){
      Homie.getLogger() << "Sensor1 is Low" << endl;
      sensorNode1.setProperty("state").send("off");
     }
     //Homie.getLogger() << "Light is now " << (lightValue ? "on" : "off") << endl;
     lastSensorValue1 = sensorValue1;
  }
  if ((sensorValue2 != lastSensorValue2)) {  // && (off == true
     if (sensorValue2 == 1){
      Homie.getLogger() << "Sensor2 is High" << endl;
      //mySwitch.switchOff('a', 1, 1); 
      //mySwitch.switchOn('a', 1, 1);
      sensorNode2.setProperty("state").send("on");
      //funkFNode.setProperty("dev").setRange(1).send("on");
     }
     else if (sensorValue2 == 0){
      Homie.getLogger() << "Sensor2 is Low" << endl;
      sensorNode2.setProperty("state").send("off");
     }
     //Homie.getLogger() << "Light is now " << (lightValue ? "on" : "off") << endl;
     lastSensorValue2 = sensorValue2;
  }
  
  if (lightValue != lastLightValue) {
     if (lightValue == 0){
      Homie.getLogger() << "Light is on" << endl;
      lightNode.setProperty("state").send("on");
      off = false;
     }
     else if (lightValue == 1){
      Homie.getLogger() << "Light is off" << endl;
      lightNode.setProperty("state").send("off");
      off = true;
     } 
     //Homie.getLogger() << "Light is now " << (lightValue ? "on" : "off") << endl;
     //lightNode.setProperty("open").send(lightValue ? "true" : "false");
     lastLightValue = lightValue;
  }
  //Homie.getLogger() << (sensorValue) << endl;  // light on: 0 light off: 1
  if (soundValue != lastSoundValue) {
     if (soundValue == 0){
      Homie.getLogger() << "Sound is High" << endl;
      soundNode.setProperty("state").send("on");
     }
     else if (soundValue == 1){
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
    temperature = temperature-3.4; // +- Offset
    //humidity= humidity; // +- Offset
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
/*
bool funkFHandler(const HomieRange& range, const String& value) {
  if (!range.isRange) return false;  // if it's not a range
  if (range.index < 1 || range.index > NUMBER_OF_LED) return false;  // if it's not a valid range
  if (value != "on" && value != "off") return false;  // if the value is not valid
  bool on;
  if (range.index == 1){ // 
    if (on = (value == "on")){
      //Homie.getLogger() << "ist 5/on" << endl;
      mySwitch.switchOff('a', 1, 1); 
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
*/
void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  mySwitch.enableTransmit(FunkPin);
  dht.begin();  
  
  pinMode(PIN_SOUND, INPUT);
  pinMode(PIN_MOTION1, INPUT);
  pinMode(PIN_MOTION2, INPUT);
  pinMode(PIN_LIGHT, INPUT);
  debouncermotion1.attach(PIN_MOTION1);
  debouncermotion2.attach(PIN_MOTION2);
  debouncerlight.attach(PIN_LIGHT);
  debouncersound.attach(PIN_SOUND);

  debouncermotion1.interval(1000);
  debouncermotion2.interval(1000);
  debouncerlight.interval(60);
  debouncersound.interval(10);

  Homie_setFirmware("awesome-Flur", "1.0.0");

  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  
  //funkANode.advertiseRange("dev", 1, NUMBER_OF_LED).settable(funkAHandler);
  //funkCNode.advertiseRange("dev", 1, NUMBER_OF_LED).settable(funkCHandler);
  //funkGNode.advertiseRange("dev", 1, NUMBER_OF_LED).settable(funkGHandler);
  //funkFNode.advertiseRange("dev", 1, NUMBER_OF_LED).settable(funkFHandler);
  //funkPNode.advertiseRange("dev", 1, NUMBER_OF_LED).settable(funkPHandler);
  
  temperatureNode.advertise("unit");
  temperatureNode.advertise("degrees");
  humidityNode.advertise("unit");
  humidityNode.advertise("degrees");
  
  soundNode.advertise("state");
  sensorNode1.advertise("state"); 
  sensorNode2.advertise("state");   
  lightNode.advertise("state");

  //Homie.reset();
  //Homie.disableLedFeedback(); // Disable Onboard LED to use TX
  Homie.disableResetTrigger(); // disable GPIO 0 as a reset
  Homie.setup();
}

void loop() {
  Homie.loop();
  debouncersound.update();
  debouncermotion1.update();
  debouncermotion2.update();
  debouncerlight.update();
}
