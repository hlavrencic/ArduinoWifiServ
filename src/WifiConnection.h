#ifndef H_WIFI_CONNECTION

    #define H_WIFI_CONNECTION

    #include <DNSServer.h>

    #ifdef ARDUINO_ARCH_ESP32
        #include <WiFi.h>
    #else
        #include <ESP8266WiFi.h>
    #endif

    enum WifiConnectionStatus {
        NONE = 0,
        DISCONNECTED = 1,
        CONNECTING = 2,
        CONNECTED = 3
    };

    class WifiConnection {
    public:
        static String IpAddress2String(const IPAddress& ipAddress)
        {
            return String(ipAddress[0]) + String(".") +\
            String(ipAddress[1]) + String(".") +\
            String(ipAddress[2]) + String(".") +\
            String(ipAddress[3])  ; 
        }

        String init(const String ssidAP){
            #ifdef ARDUINO_ARCH_ESP8266
                onSoftAPModeStationConnected = WiFi.onSoftAPModeStationConnected([&](const WiFiEventSoftAPModeStationConnected &evt){
                    aid = evt.aid;
                });

                onSoftAPModeStationDisconnected = WiFi.onSoftAPModeStationDisconnected([&](const WiFiEventSoftAPModeStationDisconnected &evt){
                    aid -= 1;
                });

                onStationModeConnected = WiFi.onStationModeConnected([&](const WiFiEventStationModeConnected &evt){
                    status = WifiConnectionStatus::CONNECTED;
                });

                onStationModeDisconnected = WiFi.onStationModeDisconnected([&](const WiFiEventStationModeDisconnected &evt){
                    if(evt.reason == 1) return; // Evito actualizar la razon, luego de una desconexion manual.
                    reason = evt.reason;
                });
            #endif

            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP(ssidAP.c_str());        
        
            auto ipAP = WiFi.softAPIP();
            if(!_dNSServer.start(53, "*", ipAP)){
                Serial.println("Fallo DNS");
                return "";
            }

            return IpAddress2String(ipAP);
        };

        String getScan(){
            return lastScanJson;
        }

        void connect(String ssidNew, String pass){ 
            status = WifiConnectionStatus::CONNECTING;
            reason = 1;
            #ifdef ARDUINO_ARCH_ESP8266
                WiFi.begin(ssidNew, pass);
            #else
                WiFi.begin(ssidNew.c_str(), pass.c_str());
            #endif
            
        };

        void disconnect(){
            status = WifiConnectionStatus::DISCONNECTED;
            WiFi.disconnect();
        }

        void scan(){
            lastScanJson = "{}";
            
            #ifdef ARDUINO_ARCH_ESP8266
                WiFi.scanNetworksAsync([&](int numSsid){
                    lastScanJson = createJsonTxt([&](DynamicJsonDocument &doc){
                        for(auto i = 0; i < numSsid; i++){
                            String ssid;
                            byte enc;
                            int rss;
                            byte* bssid;
                            int channel;
                            bool hidden;
                            WiFi.getNetworkInfo(i, ssid, enc, rss, bssid, channel, hidden);

                            doc["scan"][i]["ssid"] = ssid;
                            doc["scan"][i]["enc"] = enc;
                            doc["scan"][i]["rss"] = rss;
                            doc["scan"][i]["channel"] = channel;
                            doc["scan"][i]["hidden"] = hidden;
                        }

                        WiFi.scanDelete();   
                    });
                });
            #else
                // TODO Scan
            #endif
            
        }

        void next(){
            _dNSServer.processNextRequest();

            #ifdef ARDUINO_ARCH_ESP8266
                if(status == CONNECTING){
                    switch(reason){
                        case WIFI_DISCONNECT_REASON_AUTH_EXPIRE:
                        case WIFI_DISCONNECT_REASON_ASSOC_EXPIRE:
                        case WIFI_DISCONNECT_REASON_NOT_AUTHED:
                        case WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED:
                        case WIFI_DISCONNECT_REASON_MIC_FAILURE:
                        case WIFI_DISCONNECT_REASON_BEACON_TIMEOUT:
                        case WIFI_DISCONNECT_REASON_NO_AP_FOUND:
                        case WIFI_DISCONNECT_REASON_AUTH_FAIL:
                        case WIFI_DISCONNECT_REASON_ASSOC_FAIL:
                        case WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT:
                            disconnect();                    
                            break;
                        default:
                            break;
                    };  
                }
            #endif
        };

        String getStatus(){
            auto lastState = createJsonTxt([&](DynamicJsonDocument &doc){
                auto wifiStatus = WiFi.status();
                if(wifiStatus == wl_status_t::WL_CONNECTED){
                    status = WifiConnectionStatus::CONNECTED;
                }

                doc["wifiStatus"] = wifiStatus;
                doc["gw"] = IpAddress2String(WiFi.gatewayIP());
                doc["mask"] = IpAddress2String(WiFi.subnetMask());
                doc["channel"] = WiFi.channel();
                doc["ip"] = IpAddress2String(WiFi.localIP());
                doc["aid"] = aid;
                doc["ssid"] = WiFi.SSID();
                doc["status"] = status;
                doc["reason"] = reason;
            });

            return lastState; 
        }

    private:
        DNSServer _dNSServer;

        #ifdef ARDUINO_ARCH_ESP8266
            WiFiEventHandler 
                onSoftAPModeStationConnected, 
                onSoftAPModeStationDisconnected,
                onStationModeConnected, 
                onStationModeDisconnected;

            
        #endif

        uint8_t reason = 1;
        String lastScanJson = "{}";;
        WifiConnectionStatus status = WifiConnectionStatus::NONE;

        uint8_t aid;
    };

#endif
