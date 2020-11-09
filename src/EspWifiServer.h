#include <StaticWebServer.h>
#include <WifiConnection.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

StaticWebServer staticWebServer;
WifiConnection wifiConnection;
WebSocketsServer webSocket = WebSocketsServer(81);

typedef std::function<void(String uri, JsonObject& doc)> TextReceivedHandler;

class EspWifiServer {
    public:
        bool init(const String ssid){
            auto ip = wifiConnection.init(ssid);
            staticWebServer.begin(ip);
            webSocket.begin();
            
            webSocket.onEvent([&](uint8_t num, WStype_t type, uint8_t * payload, size_t length){
                Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
                
                switch(type) {
                case WStype_DISCONNECTED:
                    Serial.printf("[%u] Disconnected!\r\n", num);
                    break;
                case WStype_CONNECTED:
                    {
                        IPAddress ipAd = webSocket.remoteIP(num);
                        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ipAd[0], ipAd[1], ipAd[2], ipAd[3], payload);
                    }
                    break;
                case WStype_TEXT:
                    {
                        Serial.printf("[%u] get Text: %s\r\n", num, payload);
                        String data = String((char *)payload);
                        DynamicJsonBuffer jsonBuffer;
                        JsonObject& json = jsonBuffer.parseObject(data);

                        if(!handler){
                            return;
                        }

                        handler(data, json);
                    }
                    break;
           
                default:
                    Serial.printf("Invalid WStype [%d]\r\n", type);
                    break;
                }
            });

            return true;
        }

        String connect(const char* ssid, const char* pass){
            return wifiConnection.connect(ssid, pass);
        }

        void send(std::function<void(JsonObject& doc)> func){
            auto jsonTxt = createJsonTxt(func);
            webSocket.broadcastTXT(jsonTxt);
        }

        void next(){
            wifiConnection.next();
            staticWebServer.handleClient();
            webSocket.loop();
        }

        TextReceivedHandler handler;
    
    private:
        
        String createJsonTxt(std::function<void(JsonObject& doc)> func){
            DynamicJsonBuffer jsonBuffer;
            JsonObject& json = jsonBuffer.createObject();
            func(json);
            String jsonTxt;
            json.printTo(jsonTxt);
            return jsonTxt;
        }
};