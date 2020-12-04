#ifndef H_StaticWebServer
    #define H_StaticWebServer

    #include <ArduinoJsonPlus.h>

    #ifdef ARDUINO_ARCH_ESP32
        #include <WebServer.h>
        WebServer server(80);
    #else
        #include <ESP8266WebServer.h>
        ESP8266WebServer server(80);
    #endif

    class StaticWebServer{
        public:
            bool begin(String ip, FS fs){
                

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
                    indexPath.concat("192.168.4.1");
                    indexPath.concat("/index.html");
                    server.sendHeader("Location", indexPath);
                    server.send(303);
                });

                server.serveStatic("/", fs, "/");

                server.begin();

                return true;
            };

            void handleClient(){
                server.handleClient();
            };

            void handlePostRequest(String uri, WebServer::THandlerFunction callback){
                server.on(uri, HTTP_POST, callback);
            }

            String send(std::function<void (DynamicJsonDocument &doc)> jsonFunc){
                DynamicJsonDocument doc(1024);
                auto jsonTxt = createJsonTxt(jsonFunc);
                send(jsonTxt);
                return jsonTxt;
            }   

            void send(String json){
                server.send(200, "application/json", json);
            }   
    };
#endif

