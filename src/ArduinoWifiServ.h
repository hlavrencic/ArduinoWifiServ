#include <StaticWebServer.h>
#include <WifiConnection.h>
#include <ArduinoJsonPlus.h>
#include <WebSocketServerJson.h>
#include <ServerFunctions.h>

StaticWebServer staticWebServer;
WebSocketServerJson webSocketServerJson;
WifiConnection wifiConnection;

ServerFunctions serverFunctions(&staticWebServer, &wifiConnection);

class ArduinoWifiServ {
    public:
        bool init(const String ssid, FS fs){
            auto ip = wifiConnection.init(ssid);
            staticWebServer.begin(ip, fs);
            webSocketServerJson.begin();
            serverFunctions.load();
            return true;
        }

        bool next(){
            auto m = micros();

            wifiConnection.next();
            staticWebServer.handleClient();
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

            return (time > 10000);
        }
    
    private:
        unsigned long lastPrint = 0;
        unsigned long maxTime = 0;    
};