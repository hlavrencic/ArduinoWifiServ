#include <Arduino.h>
#include <EspWifiServer.h>
#include <ServerFunctions.h>

EspWifiServer espWifiServer;
WebSocketServerJson webSocketServerJson;
ServerFunctions serverFunctions(&webSocketServerJson, &staticWebServer, &wifiConnection);

const uint8_t LED_PIN = 2;



void setup(){
    Serial.begin(230400);
    
    delay(1000);

    pinMode(LED_PIN, OUTPUT); // LED PIN;
   
    espWifiServer.init("My Access Point");
    
    webSocketServerJson.begin();
    serverFunctions.load();
  
    Serial.println("FIN SETUP");
}

unsigned long lastPrint = 0;

unsigned long maxTime = 0;

void loop(){
    auto m = micros();
        
    espWifiServer.next();
    webSocketServerJson.loop();


    auto time = micros() - m;
    if(maxTime < time){
        maxTime = time;
    }

    if(maxTime > 10000 && m - lastPrint > 1000000){
        lastPrint = m;
        Serial.printf(" %lu ", maxTime);
        maxTime = 0;
    }
}