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
                auto ssid = server.arg("ssid");
                auto password = server.arg("password");
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

        WiFiEventHandler onStationModeGotIP, onSoftAPModeStationConnected;        
};

ServerFunctions::ServerFunctions(
    StaticWebServer *_staticWebServer,
    WifiConnection *_wifiConnection){

    staticWebServer = _staticWebServer;
    wifiConnection = _wifiConnection;
}