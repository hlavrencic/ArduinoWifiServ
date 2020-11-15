#include <DNSServer.h>
#include <ESP8266WiFi.h>

class WifiConnection {
public:
    static String IpAddress2String(const IPAddress& ipAddress)
    {
        return String(ipAddress[0]) + String(".") +\
        String(ipAddress[1]) + String(".") +\
        String(ipAddress[2]) + String(".") +\
        String(ipAddress[3])  ; 
    }

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

        return IpAddress2String(ip);
    };

    String getScan(){
        return lastScanJson;
    }

    void connect(String ssid, String pass){   
        wifiStatus = "CONNECTING";
        WiFi.begin(ssid, pass);
        lastStateChange = millis();
    };

    void scan(){
        lastScanJson = "{}";
        WiFi.scanNetworksAsync([&](int numSsid){
            lastScanJson = createJsonTxt([&](DynamicJsonDocument &doc){
                for(auto i = 0; i < numSsid; i++){
                    String ssid;
                    byte enc;
                    int rss;
                    byte* bssid;
                    int channel;
                    bool hidden;
                    WiFi.getNetworkInfo(i, ssid, enc, rss, bssid, channel, hidden);

                    doc["scan"][i]["ssid"] = ssid;
                    doc["scan"][i]["enc"] = enc;
                    doc["scan"][i]["rss"] = rss;
                    doc["scan"][i]["channel"] = channel;
                    doc["scan"][i]["hidden"] = hidden;
                }

                WiFi.scanDelete();   
            });
        });
    }

    void next(){
        _dNSServer.processNextRequest();
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

    String lastState;
    unsigned long lastSend = 0;
    unsigned long lastStateChange = 0;
    bool sendStatusSwitch;
    String lastScanJson = "{}";;

    String ip, gw, mask, channel, wifiStatus, reason, ssid;
    uint8_t aid;
};