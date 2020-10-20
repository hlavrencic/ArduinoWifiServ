# ArduinoWifiServ

Access and control your ESP from any mobile
The library configures and Acces Point that redirects your mobile automatically to a WebSite hosted inside the ESP. After the conection is stablished, the comunication between your mobile and ESP is bidireccional. That allows ESP or Mobile to send data anytime.

![](https://raw.githubusercontent.com/hlavrencic/imageHost/master/ArduinoWifiServ/index1.png)

## How connection works
The library configures an Access Point with a DNS host that redirects any WiFi client to ESP WebSite home page.


## The WebSite
The library includes some WebSite files (index, images and some javascript) that you must upload to your ESP FS/SPIFFS. You can add your own files to the website and consume the existing functionality to send and receive data from your ESP.

## Send data to ESP
There is and embebed functionality in the .js files that allows you to send any Form data to ESP. Then you just need to suscribe an event in your ESP to wait for the data arrival.

You only need to create the form declaring a name for each input. Those will arrive at ESP as an array of tuples (key, value).

## Send data from ESP 
Within the library, you will find some functions to send an array of tuples directly to all browser clients. This is all done by WebSockets. To capture and show this values in the web page, you just need to add some attributes to de tag elements on witch you want the value to be displayed.``````
