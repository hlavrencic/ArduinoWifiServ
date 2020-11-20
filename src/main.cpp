#include <Arduino.h>
#include <FS.h>
#include <EspWifiServer.h>


EspWifiServer espWifiServer;

const uint8_t LED_PIN = 5;

void setup(){
    pinMode(LED_PIN, OUTPUT); // LED PIN;
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(230400);
    while (!Serial)
    {
        delay(100);
    }

    if(!SPIFFS.begin()){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    
    
    espWifiServer.init("My Access Point", SPIFFS);

    Serial.println("FIN SETUP");
}

void loop(){
    auto isDelayed = espWifiServer.next();
    digitalWrite(LED_BUILTIN, isDelayed);
}