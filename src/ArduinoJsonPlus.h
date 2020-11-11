#ifndef H_ArduinoJsonPlus
    #define H_ArduinoJsonPlus

    #include <ArduinoJson.h>

    typedef std::function<void(DynamicJsonDocument &doc)> WebSocketJsonHandler;

    String createJsonTxt(WebSocketJsonHandler func){
        DynamicJsonDocument doc(1024);
        func(doc);
        String jsonTxt;
        serializeJson(doc, jsonTxt);
        return jsonTxt;
    }

#endif