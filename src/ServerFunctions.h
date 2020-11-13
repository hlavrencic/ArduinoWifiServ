#include <WebSocketServerJson.h>

class ServerFunctions {
    public:
        ServerFunctions(
            WebSocketServerJson *webSocketServerJson,
            StaticWebServer *staticWebServer,
            WifiConnection *wifiConnection);

        void load(){
            webSocketServerJson->addHandler([&](DynamicJsonDocument &doc){
                if(doc.containsKey("SCAN_WIFI")){
                    getScanAsync();
                } 
                else if (doc.containsKey("ssid"))
                {
                    auto ssid = doc["ssid"];
                    auto password = doc["password"];

                    wifiConnection->connect(ssid, password);                    
                }
                
            });

            
            server.on("/scan", HTTP_POST, [&](){
                postScanAsync();
            });

            server.on("/connect", HTTP_POST, [&](){                
                auto ssid = server.arg("ssid");
                auto password = server.arg("password");
                
                wifiConnection->connect(ssid, password);
            });

            server.on("/disconnect", HTTP_POST, [&](){
                Serial.println("DISCONNECTING");
                WiFi.disconnect();
            });
        }

        void getScanAsync(){
            WiFi.scanNetworksAsync([&](int numSsid){
                webSocketServerJson->send([&](DynamicJsonDocument &doc){
                    for(auto i = 0; i < numSsid; i++){
                        String ssid;
                        byte enc;
                        int rss;
                        byte* bssid;
                        int channel;
                        bool hidden;
                        WiFi.getNetworkInfo(i, ssid, enc, rss, bssid, channel, hidden);

                        doc["scan"][i]["ssid"] = ssid;
                    }
                });

                WiFi.scanDelete();        
            });
        }  

        void postScanAsync(){
            WiFi.scanNetworksAsync([&](int numSsid){
                staticWebServer->send([&](DynamicJsonDocument &doc){
                    for(auto i = 0; i < numSsid; i++){
                        String ssid;
                        byte enc;
                        int rss;
                        byte* bssid;
                        int channel;
                        bool hidden;
                        WiFi.getNetworkInfo(i, ssid, enc, rss, bssid, channel, hidden);

                        doc["scan"][i]["ssid"] = ssid;
                    }
                });

                WiFi.scanDelete();        
            });
        }           

    private:
        WebSocketServerJson *webSocketServerJson;
        StaticWebServer *staticWebServer;
        WifiConnection *wifiConnection;

        WiFiEventHandler onStationModeGotIP, onSoftAPModeStationConnected;        
};

ServerFunctions::ServerFunctions(
    WebSocketServerJson *_webSocketServerJson, 
    StaticWebServer *_staticWebServer,
    WifiConnection *_wifiConnection){

    webSocketServerJson = _webSocketServerJson;
    staticWebServer = _staticWebServer;
    wifiConnection = _wifiConnection;
}