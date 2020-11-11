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
            });

            
            server.on("/scan", HTTP_POST, [&](){
                postScanAsync();
            });

            server.on("/connect", HTTP_POST, [&](){
                Serial.println("CONNECTING");
                wifiConnection->connect("HUAWEI-165B", "marcopolo12", [&](String ip){
                    staticWebServer->send([&](DynamicJsonDocument &doc){
                        doc["ip"] = ip;
                    });
                });
            });
        }

        void getScanAsync(){
            WiFi.scanNetworksAsync([&](int numSsid){
                Serial.printf("Found %u SSID", numSsid);

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

                Serial.println("Eliminando scan...");
                WiFi.scanDelete();        
            });
        }  

        void postScanAsync(){
            WiFi.scanNetworksAsync([&](int numSsid){
                Serial.printf("Found %u SSID", numSsid);

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

                Serial.println("Eliminando scan...");
                WiFi.scanDelete();        
            });
        }           

    private:
        WebSocketServerJson *webSocketServerJson;
        StaticWebServer *staticWebServer;
        WifiConnection *wifiConnection;
        
};

ServerFunctions::ServerFunctions(
    WebSocketServerJson *_webSocketServerJson, 
    StaticWebServer *_staticWebServer,
    WifiConnection *_wifiConnection){

    webSocketServerJson = _webSocketServerJson;
    staticWebServer = _staticWebServer;
    wifiConnection = _wifiConnection;
}