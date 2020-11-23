class ServerFunctions {
    public:
        ServerFunctions(
            StaticWebServer *staticWebServer,
            WifiConnection *wifiConnection);

        void load(){           
            staticWebServer->handlePostRequest("/startScan", [&](){
                staticWebServer->send("{}");
                wifiConnection->scan();
            });

            staticWebServer->handlePostRequest("/getScan", [&](){   
                auto jsonTxt = wifiConnection->getScan();
                staticWebServer->send(jsonTxt);
            });

            staticWebServer->handlePostRequest("/connect", [&](){    
                String ssid;
                String password;
                
                if(server.hasArg("ssid")) {
                    ssid = server.arg("ssid");
                    password = server.arg("password");
                } else {
                    auto arg0 =server.arg(0);
                    parseJsonTxt(arg0, [&](DynamicJsonDocument &doc){
                        ssid = doc["ssid"].as<String>();
                        password = doc["password"].as<String>();
                    });
                }

                wifiConnection->connect(ssid, password);
                staticWebServer->send("{}");
            });

            server.on("/disconnect", HTTP_POST, [&](){
                Serial.println("DISCONNECTING");
                staticWebServer->send("{}");
                WiFi.disconnect();
            });

            server.on("/wifiStatus", HTTP_POST, [&](){
                auto jsonTxt = wifiConnection->getStatus();
                staticWebServer->send(jsonTxt);
            });
        }        

    private:
        StaticWebServer *staticWebServer;
        WifiConnection *wifiConnection;
};

ServerFunctions::ServerFunctions(
    StaticWebServer *_staticWebServer,
    WifiConnection *_wifiConnection){

    staticWebServer = _staticWebServer;
    wifiConnection = _wifiConnection;
}