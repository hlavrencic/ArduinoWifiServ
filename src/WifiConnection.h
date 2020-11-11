#include <DNSServer.h>
#include <ESP8266WiFi.h>

class WifiConnection {
public:
    String init(const String ssid){
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(ssid);        

        auto ip = WiFi.softAPIP();
        Serial.print("IP: ");
        Serial.println(ip);

        if(!_dNSServer.start(53, "*", ip)){
            Serial.println("Fallo DNS");
            return "";
        }

        WiFi.onStationModeConnected([](const WiFiEventStationModeConnected &event){
            Serial.print("CONNECTED");
        });

        WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &event){
            Serial.print("DISCONNECTED");
        });

        return IpAddress2String(ip);
    };

    String connect(const char* ssid, const char* pass){

        WiFi.begin(ssid, pass);
        Serial.println();
        Serial.print("Connecting to WiFi... ");
        Serial.print(ssid);
        Serial.print("@");
        Serial.println(pass);

        auto timeout = millis();

        while(WiFi.status() != WL_CONNECTED){
            delay(500);
            Serial.print(".");
            if(millis() - timeout  > 100000){
                Serial.print("TIMEOUT");
                return "";
            }
        }

        auto ip = WiFi.localIP();
        Serial.print("IP: ");
        Serial.println(ip);
        
        WiFi.setAutoReconnect(true);
        
        return IpAddress2String(ip);
    }

    void next(){
        _dNSServer.processNextRequest();
    };

private:
    DNSServer _dNSServer;

    // author apicquot from https://forum.arduino.cc/index.php?topic=228884.0
    String IpAddress2String(const IPAddress& ipAddress)
    {
        return String(ipAddress[0]) + String(".") +\
        String(ipAddress[1]) + String(".") +\
        String(ipAddress[2]) + String(".") +\
        String(ipAddress[3])  ; 
    }
};