#ifndef H_RedirectInvalidHostHandler
    #define H_RedirectInvalidHostHandler

    #include <ESPAsyncWebServer.h> 
    #include <wifiServ.h>   
    
    class RedirectInvalidHostHandler : public AsyncWebHandler {
    public:
        RedirectInvalidHostHandler(WifiServ* wifiServ) {
            _wifiServ = wifiServ;
        }
        virtual ~RedirectInvalidHostHandler() {}

        bool canHandle(AsyncWebServerRequest *request){   
            auto hostValue = request->host();
            //auto ip = _wifiServ->GetIP().toString();
            //auto hostValido = hostValue.compareTo("connectivitycheck.gstatic.com")==0 || hostValue.compareTo(ip)==0;

            auto hostValido = true;

            if(!hostValido){
                Serial.print("HOST INVALIDO: ");
                Serial.println(hostValue);
            }

            return !hostValido;
        }

        void handleRequest(AsyncWebServerRequest *request) {
            if(request->method() == HTTP_GET) {
                request->redirect("http://connectivitycheck.gstatic.com/generate_204");
            } else {
                request->send(404);
            }    
        }
    private:
        WifiServ* _wifiServ;
    };

#endif