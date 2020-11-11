#include <WebSocketServerJson.h>

class ServerFunctions {
    public:
        ServerFunctions(WebSocketServerJson *webSocketServerJson);

        void load(){
            webSocketServerJson->addHandler([&](DynamicJsonDocument &doc){
                if(doc.containsKey("SCAN_WIFI")){
                    getScanAsync();
                }
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

        void socketHandler(DynamicJsonDocument &doc){
            
        }    

    private:
        WebSocketServerJson *webSocketServerJson;

        
};

ServerFunctions::ServerFunctions(WebSocketServerJson *_webSocketServerJson){
    webSocketServerJson = _webSocketServerJson;
}