#include <StaticWebServer.h>
#include <WifiConnection.h>
#include <ArduinoJsonPlus.h>

StaticWebServer staticWebServer;
WifiConnection wifiConnection;

class EspWifiServer {
    public:
        bool init(const String ssid){
            auto ip = wifiConnection.init(ssid);
            staticWebServer.begin(ip);
            return true;
        }

        void next(){
            wifiConnection.next();
            staticWebServer.handleClient();
        }
    
};