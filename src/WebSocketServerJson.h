#include <WebSocketsServer.h>
//#include <ArduinoJson.h>

WebSocketsServer webSocket = WebSocketsServer(81);

typedef std::function<void(DynamicJsonDocument &doc)> WebSocketJsonHandler;

class WebSocketServerJson {
    public:
        void begin(){
            webSocket.begin();
        }

        void addHandler(std::function<void(DynamicJsonDocument &doc)> handler){
            handlerArray.push_back(handler);

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
                        DynamicJsonDocument doc(1024);
                        deserializeJson(doc, data);
                        
                        for (std::vector<WebSocketJsonHandler>::iterator it = handlerArray.begin() ; it != handlerArray.end(); ++it){
                            WebSocketJsonHandler handlerPointer = *it;

                            handlerPointer(doc);
                        }
                    }
                    break;
           
                default:
                    Serial.printf("Invalid WStype [%d]\r\n", type);
                    break;
                }
            });
        }

        void send(std::function<void(DynamicJsonDocument& doc)> func){
            auto jsonTxt = createJsonTxt(func);
            webSocket.broadcastTXT(jsonTxt);
        }

        void loop(){
            webSocket.loop();
        }

    private:
          
        std::vector<WebSocketJsonHandler> handlerArray;

        String createJsonTxt(std::function<void(DynamicJsonDocument& doc)> func){
            DynamicJsonDocument doc(1024);
            func(doc);
            String jsonTxt;
            serializeJson(doc, jsonTxt);
            return jsonTxt;
        }

        
};