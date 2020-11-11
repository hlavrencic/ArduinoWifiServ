#include <StaticWebServer.h>
#include <WifiConnection.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

StaticWebServer staticWebServer;
WifiConnection wifiConnection;

class EspWifiServer {
    public:
        bool init(const String ssid){
            auto ip = wifiConnection.init(ssid);
            staticWebServer.begin(ip);
            return true;
        }

        String connect(const char* ssid, const char* pass){
            return wifiConnection.connect(ssid, pass);
        }

        void next(){
            wifiConnection.next();
            staticWebServer.handleClient();
        }

        void handlePostRequest(const Uri &uri, std::function<void (DynamicJsonDocument &doc)> callback){
            server.on(uri, HTTP_POST, [&](){
                DynamicJsonDocument doc(1024);
                auto jsonTxt = createJsonTxt(callback);
                server.send(200, "application/json", jsonTxt);
            });
        }
    
    private:
        
        String createJsonTxt(std::function<void(DynamicJsonDocument& doc)> func){
            DynamicJsonDocument doc(1024);
            func(doc);
            String jsonTxt;
            serializeJson(doc, jsonTxt);
            return jsonTxt;
        }
};