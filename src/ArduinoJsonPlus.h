#ifndef H_ArduinoJsonPlus
    #define H_ArduinoJsonPlus

    #include <ArduinoJson.h>

    typedef std::function<void(DynamicJsonDocument &doc)> WebSocketJsonHandler;

    String createJsonTxt(WebSocketJsonHandler func){
        DynamicJsonDocument doc(1024);
        func(doc);
        String jsonTxt;
        serializeJson(doc, jsonTxt);
        doc.clear();
        doc.garbageCollect();
        return jsonTxt;
    }

    void parseJsonTxt(String jsonTxt, WebSocketJsonHandler func){
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, jsonTxt);
        func(doc);
        doc.clear();
        doc.garbageCollect();
    }
#endif