#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Homie.h>


const int PIN_MOTION =1; 
const int PIN_LIGHT = 3; //Rx  
const int PIN_TEMP=2;
int lastSensorValue=-1;
int lastLightValue = -1;  

const int TEMPERATURE_INTERVAL = 120; // time [sec] between measurements 
unsigned long lastTemperatureSent = 0;

float humidity, temperature;

#define DHTTYPE DHT22                                      
DHT dht(PIN_TEMP, DHTTYPE);

 // Bounce is built into Homie, so you can use it without including it first
Bounce debouncerlight = Bounce();
//Bounce debouncermotion = Bounce();



//HomieNode sensorNode("motion", "door");
HomieNode lightNode("light", "door");
HomieNode temperatureNode("temperature", "temperature");
HomieNode humidityNode("humidity", "temperature");


void setupHandler() {
  temperatureNode.setProperty("unit").send("c");
  humidityNode.setProperty("unit").send("%");
}

void loopHandler() {
  //int sensorValue = debouncermotion.read();
  int lightValue = debouncerlight.read();
  //Homie.getLogger() << (sensorValue) << endl;  
  /*
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
  */
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

  if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL * 1000UL || lastTemperatureSent == 0) {
    //float temperature = 22; // Fake temperature here, for the example
    humidity = dht.readHumidity();                           
    temperature = dht.readTemperature();
    temperature = temperature-8.0; // Offset 2
    humidity = humidity+4; // Offset
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


void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  dht.begin();  
  
  pinMode(PIN_MOTION, INPUT);
  pinMode(PIN_LIGHT, INPUT);
//  debouncermotion.attach(PIN_MOTION);
  debouncerlight.attach(PIN_LIGHT);

  //debouncermotion.interval(5000);
  debouncerlight.interval(60);

  Homie_setFirmware("ESP8266-Homie-Pott", "1.0.0");

  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
 
  
  temperatureNode.advertise("unit");
  temperatureNode.advertise("degrees");
  humidityNode.advertise("unit");
  humidityNode.advertise("degrees");
  
  //sensorNode.advertise("state");   
  lightNode.advertise("state");

  //Homie.reset();
  Homie.disableLedFeedback(); // Disable Onboard LED to use TX
  Homie.disableResetTrigger(); // disable GPIO 0 as a reset
  Homie.setup();
}

void loop() {
  Homie.loop();
  //debouncermotion.update();
  debouncerlight.update();
}
