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
        });

        onSoftAPModeStationDisconnected = WiFi.onSoftAPModeStationDisconnected([&](const WiFiEventSoftAPModeStationDisconnected &evt){
            aid -= 1;
        });

        onStationModeDisconnected = WiFi.onStationModeDisconnected([&](const WiFiEventStationModeDisconnected &evt){
            reason = evt.reason;
        });

        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(ssidAP);        
    
        auto ipAP = WiFi.softAPIP();
        if(!_dNSServer.start(53, "*", ipAP)){
            Serial.println("Fallo DNS");
            return "";
        }

        return IpAddress2String(ipAP);
    };

    String getScan(){
        return lastScanJson;
    }

    void connect(String ssidNew, String pass){   
        reason = "";
        WiFi.begin(ssidNew, pass);
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
        auto lastState = createJsonTxt([&](DynamicJsonDocument &doc){
            doc["wifiStatus"] = WiFi.status();
            doc["reason"] = reason;
            doc["gw"] = IpAddress2String(WiFi.gatewayIP());
            doc["mask"] = IpAddress2String(WiFi.subnetMask());
            doc["channel"] = WiFi.channel();
            doc["ip"] = IpAddress2String(WiFi.localIP());
            doc["aid"] = aid;
            doc["ssid"] = WiFi.SSID();
            doc["psk"] = WiFi.psk();
        });

        return lastState; 
    }

private:
    DNSServer _dNSServer;
    WiFiEventHandler 
        onSoftAPModeStationConnected, 
        onSoftAPModeStationDisconnected, 
        onStationModeDisconnected;

    String lastScanJson = "{}";;

    String reason;
    uint8_t aid;
};