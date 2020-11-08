#include <ESP8266WebServer.h>
#include <FS.h>

ESP8266WebServer server(80);

class StaticWebServer{
    
    public:
        bool begin(String ip){
            if(!SPIFFS.begin()){
                Serial.println("SPIFFS Mount Failed");
                return false;    
            }

            server.serveStatic("/", SPIFFS, "/");

            server.on("/generate_204", HTTP_GET, []() {
                Serial.println("Redirect /generate_204");
            });

            server.on("/", HTTP_GET, []() {
                Serial.println("Redirect /");
            });

            auto ipStr = ip.c_str();

            server.onNotFound([&](){
                Serial.print("Not Found: ");
                Serial.print(server.uri());
                Serial.print(" - ");
                Serial.print(server.method());
                Serial.print(" - ");
                Serial.print(server.args());
                Serial.print(" - ");
                Serial.print(server.client());
                Serial.print(" - ");
                Serial.println(server.hostHeader());

                String indexPath;
                indexPath.concat("http://");
                indexPath.concat(ipStr);
                indexPath.concat("/index.html");
                server.sendHeader("Location", indexPath);
                server.send(303);
            });

            server.begin();

            return true;
        };

        void handleClient(){
            server.handleClient();
        };
};