#include <StaticWebServer.h>
#include <WifiConnection.h>
#include <WebSocketsServer.h>

StaticWebServer staticWebServer;
WifiConnection wifiConnection;
WebSocketsServer webSocket = WebSocketsServer(81);

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
                        Serial.printf("[%u] get Text: %s\r\n", num, payload);
                    break;
           
                default:
                    Serial.printf("Invalid WStype [%d]\r\n", type);
                    break;
                }
            });
        }

        String connect(const char* ssid, const char* pass){
            return wifiConnection.connect(ssid, pass);
        }

        void next(){
            wifiConnection.next();
            staticWebServer.handleClient();
            webSocket.loop();
        }
};