#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoWifiServ.h>


ArduinoWifiServ ArduinoWifiServ;

const uint8_t LED_PIN = 5;

void setup(){
    pinMode(LED_PIN, OUTPUT); // LED PIN;
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(230400);

    if(!SPIFFS.begin()){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    
    
    ArduinoWifiServ.init("My Access Point", SPIFFS);

    Serial.println("FIN SETUP");
}

void loop(){
    auto isDelayed = ArduinoWifiServ.next();
    digitalWrite(LED_BUILTIN, isDelayed);
}