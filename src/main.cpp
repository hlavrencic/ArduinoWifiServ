#include <Arduino.h>
#include <EspWifiServer.h>

EspWifiServer espWifiServer;

const uint8_t LED_PIN = 2;

void setup(){
    Serial.begin(230400);

    delay(1000);

    pinMode(LED_PIN, OUTPUT); // LED PIN;

    espWifiServer.handler = [&](String uri, JsonObject& json){
        if(json.containsKey("SCAN_WIFI")){
            espWifiServer.send([&](JsonObject &json){
                json["scan"]["ssid"] = "NET1";
                json["scan"]["ssid"] = "NET1";
            });
        }
    }; 

    espWifiServer.init("My AP");
    
    auto ip = espWifiServer.connect("MARCO_POLO", "marcopolo12");
    
    /*
    wifiServ.textReceivedHandler = [&](JsonVariant doc){
        if(doc.containsKey("pin2")){
            const char* pin2 = doc["pin2"];

            Serial.println(pin2);

            auto setLedTo = strcmp(pin2, "on") == 0;

            Serial.print("LED set to: ");
            Serial.println(setLedTo);

            digitalWrite(LED_PIN, setLedTo);
        }
        
    };
    */
}

void loop(){
    espWifiServer.next();
}