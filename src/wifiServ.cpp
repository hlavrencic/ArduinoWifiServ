#include <wifiServ.h>
#include <RedirectIndexHandler.h>
#include <RedirectInvalidHostHandler.h>
#include <SPIFFSReadHandler.h>
#include <FSInclude.h>

JsonVariant WifiServ::initJson(){
    StaticJsonDocument<200> doc;
    return doc.as<JsonVariant>();
}

String WifiServ::sendJson(JsonVariant doc){
    String txt;
    serializeJson(doc, txt);
    ws.textAll(txt);
    return txt;
};

void WifiServ::serilize(char* data){
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
    } else if(jsonDoc.containsKey("SSID_AP") ){
        auto ssid = jsonDoc["SSID_AP"];
        connectAP(ssid);
    } else if(jsonDoc.containsKey("SCAN_WIFI") ){
        _scan = true;
    }



    textReceivedHandler(jsonDoc);
};

void WifiServ::onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
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

            serilize(dataChar);
        }
    }      
};

void WifiServ::connect(const char* ssid, const char* pass){
    _ssid = String(ssid);
    _pass = String(pass);
}

IPAddress WifiServ::GetIP(){
    return _ip;
}

void WifiServ::_connect(const char* ssid, const char* pass){
    
    _cleanWifi();

    WiFi.begin(ssid, pass);
    
    Serial.print("Connecting to WiFi... ");
    Serial.print(ssid);
    Serial.print("@");
    Serial.println(pass);
    auto connectStatus = WiFi.waitForConnectResult();
    if(connectStatus != WL_CONNECTED){
        Serial.println("CONNECTION FAILED.");
        connectAP("AP ESP");
        return;
    }

    _ip = WiFi.localIP();
    Serial.print("IP: ");
    Serial.println(_ip);
    
    WiFi.setAutoReconnect(true);
    
    _setup();

    server.begin();

    _estadoConexion = WifiServEstadoConexion::CONECTADO;
}

void WifiServ::connectAP(const char* ssid){
    _cleanWifi();

    WiFi.softAP(ssid);
    

    _ip = WiFi.softAPIP();
    Serial.print("IP: ");
    Serial.println(_ip);
    if(!_dNSServer.start(53, "*", _ip)){
        Serial.println("Fallo DNS");
        return;
    }

    _setup();

    server.begin();

    _estadoConexion = WifiServEstadoConexion::CONECTADO_AP;
}

void WifiServ::_cleanWifi(){
    if(!(_estadoConexion == WifiServEstadoConexion::CONECTADO || _estadoConexion == WifiServEstadoConexion::CONECTADO_AP))
        return;
    
    auto estadoOriginal = _estadoConexion;
    _estadoConexion = WifiServEstadoConexion::CONFIGURADO;
    
    ws.closeAll();
    Serial.println("Server stop...");
    //server.reset();
    //server.end();
    Serial.println("STOPPIN DNS...");
    //_dNSServer.stop();

    if(estadoOriginal == WifiServEstadoConexion::CONECTADO_AP){
        Serial.println("disconecting AP...");
        //WiFi.softAPdisconnect(true);
        
        delay(1000);
    } else if (estadoOriginal == WifiServEstadoConexion::CONECTADO){
        Serial.println("Disconecting WiFi...");
        //WiFi.disconnect(true);
        delay(1000);
    }

    
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

void WifiServ::_setup(){
    if(_estadoConexion != WifiServEstadoConexion::NINGUNO){
        return;
    }

    if(!FSInclude.begin()){
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    SPIFFSReadHandler::listDir("/");
    
    ws.onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
        onEvent(server,client,type,arg,data,len);
    });

    server.addHandler(new RedirectInvalidHostHandler(this));
    server.addHandler(&ws);
    server.on(
        "/generate_204",
        HTTP_GET,
        [](AsyncWebServerRequest * request){
            request->send(FSInclude, "/indexAP.html", "text/html");
        });  
    server.addHandler(new RedirectIndexHandler());
    server.addHandler(new SPIFFSReadHandler());

    _estadoConexion = WifiServEstadoConexion::CONFIGURADO;
};

void WifiServ::loop(){
    if(_ssid.length() > 0){
        _connect(_ssid.c_str(), _pass.c_str());
        _ssid.clear();
    }

    if(_scan){
        _scan = false;

        Serial.println("** Scan Networks **");
        int numSsid = -1;

        Serial.print("Networks: ");
        WiFi.scanNetworks(true);

        while(numSsid == -1 ){
            numSsid = WiFi.scanComplete();
            delay(100);
        }
        
        Serial.println(numSsid);

        StaticJsonDocument<300> doc;
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
        
        auto jsonTxt = sendJson(doc.as<JsonVariant>());
        Serial.println(jsonTxt); 
        WiFi.scanDelete();       
    }

    if(_estadoConexion != WifiServEstadoConexion::CONECTADO_AP) return;
    _dNSServer.processNextRequest();
};