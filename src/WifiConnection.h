#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <WebSocketServerJson.h>

class WifiConnection {
public:
    static String IpAddress2String(const IPAddress& ipAddress)
    {
        return String(ipAddress[0]) + String(".") +\
        String(ipAddress[1]) + String(".") +\
        String(ipAddress[2]) + String(".") +\
        String(ipAddress[3])  ; 
    }

    WifiConnection(WebSocketServerJson *webSocketServerJson);

    String init(const String ssidAP){
        onSoftAPModeStationConnected = WiFi.onSoftAPModeStationConnected([&](const WiFiEventSoftAPModeStationConnected &evt){
            aid = evt.aid;
            lastStateChange = millis();
        });

        onSoftAPModeStationDisconnected = WiFi.onSoftAPModeStationDisconnected([&](const WiFiEventSoftAPModeStationDisconnected &evt){
            aid -= 1;
            lastStateChange = millis();
        });

        onStationModeGotIP = WiFi.onStationModeGotIP([&](const WiFiEventStationModeGotIP &evt){
            ip = WifiConnection::IpAddress2String(evt.ip);
            gw = WifiConnection::IpAddress2String(evt.gw);
            mask = WifiConnection::IpAddress2String(evt.mask);
            wifiStatus = "CONNECTED";

            lastStateChange = millis();
        });

        onStationModeConnected = WiFi.onStationModeConnected([&](const WiFiEventStationModeConnected &evt){
            channel = evt.channel;
            ssid = evt.ssid;
            wifiStatus = "CONNECTED";

            lastStateChange = millis();
        });

        onStationModeDisconnected = WiFi.onStationModeDisconnected([&](const WiFiEventStationModeDisconnected &evt){
            reason = evt.reason;
            ssid = evt.ssid;
            wifiStatus = "DISCONNECTED";

            lastStateChange = millis();
        });

        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(ssidAP);        

        auto ip = WiFi.softAPIP();
        if(!_dNSServer.start(53, "*", ip)){
            Serial.println("Fallo DNS");
            return "";
        }

        webSocketServerJson->addHandler([&](DynamicJsonDocument &doc){
            if(doc.containsKey("sendStatusSwitch")){
                sendStatusSwitch = doc["sendStatusSwitch"] == 1;
                return true;
            }    
            
            return false;
        });

        sendStatusSwitch = true;

        return IpAddress2String(ip);
    };

    void connect(String ssid, String pass){   
        wifiStatus = "CONNECTING";
        WiFi.begin(ssid, pass);
        lastStateChange = millis();
    };

    void next(){
        _dNSServer.processNextRequest();

        if(!sendStatusSwitch){
            return;
        }

        auto m = millis();
        if(m - lastSend > 5000){
            getStatus();
            webSocketServerJson->send(lastState);
        }
    };

    String getStatus(){
        if(lastStateChange >= lastSend){
            lastState = createJsonTxt([&](DynamicJsonDocument &doc){
                doc["wifiStatus"] = wifiStatus;
                doc["reason"] = reason;
                doc["gw"] = gw;
                doc["mask"] = mask;
                doc["channel"] = channel;
                doc["ip"] = ip;
                doc["aid"] = aid;
                doc["ssid"] = ssid;
            });
        } 
        
        lastSend = millis();

        return lastState; 
    }

private:
    DNSServer _dNSServer;
    WiFiEventHandler 
        onSoftAPModeStationConnected, 
        onSoftAPModeStationDisconnected, 
        onStationModeGotIP, 
        onStationModeConnected,
        onStationModeDisconnected;

    WebSocketServerJson* webSocketServerJson;
    String lastState;
    unsigned long lastSend = 0;
    unsigned long lastStateChange = 0;
    bool sendStatusSwitch;

    String ip, gw, mask, channel, wifiStatus, reason, ssid;
    uint8_t aid;
};

WifiConnection::WifiConnection(WebSocketServerJson *_webSocketServerJson){
    webSocketServerJson = _webSocketServerJson;
}