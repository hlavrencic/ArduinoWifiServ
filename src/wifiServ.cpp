#include <wifiServ.h>
#include <RedirectIndexHandler.h>
#include <RedirectInvalidHostHandler.h>
#include <SPIFFSReadHandler.h>
#include <FSInclude.h>

StaticJsonDocument<200> getScan(){
    int numSsid = -1;

    Serial.print("Networks: ");
    WiFi.scanNetworks(true);

    while(numSsid == -1 ){
        numSsid = WiFi.scanComplete();
        delay(100);
    }
    
    Serial.println(numSsid);

    Serial.println("Generando Json...");
    StaticJsonDocument<200> doc;
    for(auto i = 0; i < numSsid; i++){
        String ssid;
        byte enc;
        int rss;
        byte* bssid;
        int channel;
        bool hidden;
        WiFi.getNetworkInfo(i, ssid, enc, rss, bssid, channel, hidden);

        doc["scan"][i]["ssid"] = ssid;
    }
    
    Serial.println("Eliminando scan...");
    WiFi.scanDelete();
    return doc;
}

JsonVariant WifiServ::initJson(){
    StaticJsonDocument<200> doc;
    return doc.as<JsonVariant>();
}

String WifiServ::sendJson(StaticJsonDocument<200> doc){
    String txt;
    serializeJson(doc, txt);
    Serial.println(txt);
    ws.textAll(txt);
    return txt;
};

void WifiServ::serilize(const char* data){
    if(!textReceivedHandler) return;

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, data);

    // Test if parsing succeeds.
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    } 
    
    auto jsonDoc = doc.as<JsonVariant>();

    if(jsonDoc.containsKey("SSID") && jsonDoc.containsKey("PASSWORD")){
        auto ssid = jsonDoc["SSID"];
        auto password = jsonDoc["PASSWORD"];
        connect(ssid, password);
    } else if(jsonDoc.containsKey("SCAN_WIFI") ){
        _scan = true;
    }

    textReceivedHandler(jsonDoc);
};

void WifiServ::_onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
    if(type == WS_EVT_CONNECT){
        
        if(connectedHandler) connectedHandler();
        Serial.println("Websocket client connection received");
    
    } else if(type == WS_EVT_DISCONNECT){
        if(disconnectedHandler) disconnectedHandler();

        Serial.println("Client disconnected");
    } 

    //data packet
    if(type != WS_EVT_DATA) return;

    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len){
        //the whole message is in a single frame and we got all of it's data
        //os_printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
        if(info->opcode == WS_TEXT){
            data[len] = 0;
            auto dataChar = (char*)data;
            _dataReceived = String(dataChar);
        }
    }      
};

IPAddress WifiServ::connect(const char* ssid, const char* pass){
    _estadoConexion = WifiServEstadoConexion::CONECTANDO;

    WiFi.begin(ssid, pass);
    Serial.println();
    Serial.print("Connecting to WiFi... ");
    Serial.print(ssid);
    Serial.print("@");
    Serial.println(pass);
    while(WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }

    auto ip = WiFi.localIP();
    Serial.print("IP: ");
    Serial.println(ip);
    
    WiFi.setAutoReconnect(true);

    _estadoConexion = WifiServEstadoConexion::CONECTADO;
    return ip;
}

void WifiServ::handleGet(const char* uri, const char* func1()){
    server.on(
        uri,
        HTTP_GET,
        [&](AsyncWebServerRequest * request){
            auto txt = func1();
            request->send(200, "text/plain", txt);
        });
}

IPAddress WifiServ::init(const char* ssid){
    if(!FSInclude.begin()){
        Serial.println("SPIFFS Mount Failed");
        return NULL;
    }

    SPIFFSReadHandler::listDir("/");
    
    ws.onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
        _onEvent(server,client,type,arg,data,len);
    });

    server.addHandler(new RedirectInvalidHostHandler(this));
    server.addHandler(&ws);
    server.on(
        "/generate_204",
        HTTP_GET,
        [](AsyncWebServerRequest * request){
            request->send(FSInclude, "/index.html", "text/html");
        });          
    server.addHandler(new RedirectIndexHandler());
    server.addHandler(new SPIFFSReadHandler());

    _estadoConexion = WifiServEstadoConexion::CONFIGURADO;

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid);
    

    auto ip = WiFi.softAPIP();
    Serial.print("IP: ");
    Serial.println(ip);
    if(!_dNSServer.start(53, "*", ip)){
        Serial.println("Fallo DNS");
        return NULL;
    }

    server.begin();

    _estadoConexion = WifiServEstadoConexion::CONECTADO_AP;
    return ip;
}

void WifiServ::loop(){
    if(_estadoConexion != WifiServEstadoConexion::CONECTADO_AP && 
    _estadoConexion != WifiServEstadoConexion::CONECTADO) 
        return;

    if(!_dataReceived.isEmpty()){
        serilize(_dataReceived.c_str());
        _dataReceived.clear();
    }

    if(_scan){
        _scan = false;

        Serial.println("** Scan Networks **");
        auto scanJson = getScan();
        
        Serial.println("Enviando Json...");
        auto jsonTxt = sendJson(scanJson);
        Serial.println(jsonTxt); 
    }

    
    _dNSServer.processNextRequest();
};