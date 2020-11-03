#include <WifiServ.h>

const uint8_t LED_PIN = 2;
WifiServ wifiServ;

void setup(){
    Serial.begin(230400);

    
    delay(1000);

    pinMode(LED_PIN, OUTPUT); // LED PIN;

    wifiServ.init("My Access Point");
    auto ip = wifiServ.connect("HUAWEI-165B", "marcopolo12");
    
    Serial.println(ip);

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
}

void loop(){
    wifiServ.loop();
}