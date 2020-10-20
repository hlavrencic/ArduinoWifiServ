#ifndef H_WIFISERV
    #define H_WIFISERV

    #include <Arduino.h>
    #include <DNSServer.h>
    #include <ESPAsyncWebServer.h>
    #include <ArduinoJson.h>

    #ifdef ARDUINO_ARCH_ESP32
        #include <WiFi.h>
        #include <AsyncTCP.h>
    #else
        #include <ESP8266WiFi.h>
        #include <ESPAsyncTCP.h>
    #endif

    typedef std::function<void(JsonVariant doc)> TextReceivedHandler;
    typedef std::function<void()> SocketStatusHandler;

    enum WifiServEstadoConexion {NINGUNO = 0, CONFIGURADO = 1, CONECTADO_AP = 2, CONECTANDO = 3, CONECTADO = 4 };

    class WifiServ
    {
    public:
        IPAddress init(const char* ssid = "ESP Acces Point");
        IPAddress connect(const char* ssid, const char* pass = (const char*)__null);
        void loop();
        JsonVariant initJson();
        String sendJson(StaticJsonDocument<200> doc);
        AsyncWebServer server = AsyncWebServer(80);
        AsyncWebSocket ws = AsyncWebSocket("/ws");
        void handleGet(const char* uri, const char* func1());

        TextReceivedHandler textReceivedHandler;
        SocketStatusHandler connectedHandler;
        SocketStatusHandler disconnectedHandler;
        
    private:
        void serilize(const char* data);
        void _onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

        DNSServer _dNSServer;
        WifiServEstadoConexion _estadoConexion = WifiServEstadoConexion::NINGUNO;
        bool _scan = false;
        String _dataReceived;
    };

#endif