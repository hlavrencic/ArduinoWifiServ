#include <DNSServer.h>
#include <ESP8266WiFi.h>

enum ConnectionStatus {
    NONE = 0,
    CONNECTING = 1,
    CONNECTED = 2,
    DISCONNECTED = 3,
    GOTIP = 4,
};

typedef std::function<void(String ipAddress)> ConnectionHandler;

class WifiConnection {
public:
    static String IpAddress2String(const IPAddress& ipAddress)
    {
        return String(ipAddress[0]) + String(".") +\
        String(ipAddress[1]) + String(".") +\
        String(ipAddress[2]) + String(".") +\
        String(ipAddress[3])  ; 
    }

    String init(const String ssid){
        onSoftAPModeStationConnected = WiFi.onSoftAPModeStationConnected([](const WiFiEventSoftAPModeStationConnected &evt){
            Serial.print("onSoftAPModeStationConnected"); Serial.println(evt.aid);
        });

        onSoftAPModeStationDisconnected = WiFi.onSoftAPModeStationDisconnected([](const WiFiEventSoftAPModeStationDisconnected &evt){
            //Serial.print("onSoftAPModeStationDisconnected");
        });

        onStationModeGotIP = onStationModeGotIP = WiFi.onStationModeGotIP([&](const WiFiEventStationModeGotIP &evt){
            connectionStatus = ConnectionStatus::GOTIP;
            Serial.println("onStationModeGotIP");
        });

        onStationModeConnected = WiFi.onStationModeConnected([&](const WiFiEventStationModeConnected &event){
            connectionStatus = ConnectionStatus::CONNECTED;
            Serial.println("onStationModeConnected");
        });

        onStationModeDisconnected = WiFi.onStationModeDisconnected([&](const WiFiEventStationModeDisconnected &event){
            connectionStatus = ConnectionStatus::DISCONNECTED;
            //Serial.print("onStationModeDisconnected");
        });

        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(ssid);        

        auto ip = WiFi.softAPIP();
        Serial.print("IP: ");
        Serial.println(ip);

        if(!_dNSServer.start(53, "*", ip)){
            Serial.println("Fallo DNS");
            return "";
        }

        return IpAddress2String(ip);
    };

    void connect(String ssid, String pass, ConnectionHandler callback){
        connectionStatus = ConnectionStatus::CONNECTING;
        connectionHandler = callback;

        lastRequestTimeout = millis();
        
        Serial.printf("Connecting: %s@%s", ssid.c_str(), pass.c_str());
        WiFi.begin(ssid, pass);
    }

    void next(){
        _dNSServer.processNextRequest();

        if(connectionStatus == ConnectionStatus::GOTIP ){
            connectionStatus = ConnectionStatus::NONE;
            auto ip = WiFi.localIP();
            auto iStr = IpAddress2String(ip);
            connectionHandler(iStr);
        }
        else if (connectionStatus != ConnectionStatus::NONE && millis() - lastRequestTimeout >= 100000)
        {
            WiFi.disconnect();
            connectionStatus = ConnectionStatus::NONE;
            connectionHandler("");
        }
    };

private:
    DNSServer _dNSServer;
    WiFiEventHandler 
        onSoftAPModeStationConnected, 
        onSoftAPModeStationDisconnected, 
        onStationModeGotIP, 
        onStationModeConnected,
        onStationModeDisconnected;

    ConnectionStatus connectionStatus = ConnectionStatus::NONE;
    ConnectionHandler connectionHandler;

    unsigned long lastRequestTimeout;
};