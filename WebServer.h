/*
#include "WiFiUtil.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef WebServer_h
#define WebServer_h

#include "Arduino.h"
#include "conf.h"

class WebServer
{
  public:
    //requires a vcc pin so that we dont have it on unnecessarily and damage the sensor due to electrolysis
    WebServer(int port);
    
  private:
    ESP8266WebServer _server;
};

#endif

WebServer::WebServer(int port)
{
  ESP8266WebServer server(port);
  _server = server;
}

void WebServer::attachHandlers()
{
  
  _server.on("/", handleRoot);
  _server.on("/take-moisture-reading", takeMoistureReadingHandler);
  _server.on("/water-plant", waterPlantHandler);
  _server.on("/take-ldr-reading", getLDRReadingHandler);
  _server.on("/take-and-process-reading", takeAndprocessReadingHandler);
  _server.onNotFound(handleNotFound);

  _server.begin();

  Serial.println("HTTP server started");
}

 asdfas
 
*/
