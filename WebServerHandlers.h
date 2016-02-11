#ifndef WebServerHandlers_h
#define WebServerHandlers_h

#include "Arduino.h"

ESP8266WebServer server = null;
MoistureSensor moistureSensor = null;
AnalogReader ldrSensor = null;
Motor waterPump = null;

void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}

void getLDRReadingHandler() {
  Serial.println("taking LDR reading: ");
  int reading = ldrSensor.takeReading(2, 100);
  Serial.println(reading);

  postDataToAPI("field2", reading);

  server.send(200, "text/plain", (String)reading);

}

void takeMoistureReadingHandler() {
  int moistureReading = getMoistureReading();

  server.send(200, "text/plain", (String)moistureReading);
}

void takeAndprocessReadingHandler() {
  takeAndprocessReading();
  server.send(200, "text/plain", "takeAndprocessReading");
}

void waterPlantHandler() {
  waterPump.activate(msWateringActiveDelay);
  server.send(200, "text/plain", "watering plant");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void attachWebServerHandlers(ESP8266WebServer s) {
  server = s;
  server.on("/", handleRoot);
  server.on("/take-moisture-reading", takeMoistureReadingHandler);
  server.on("/water-plant", waterPlantHandler);
  server.on("/take-ldr-reading", getLDRReadingHandler);
  server.on("/take-and-process-reading", takeAndprocessReadingHandler);
  server.onNotFound(handleNotFound);
}

void injectWebServerHandlerDependencies(MoistureSensor ms, AnalogReader ls, Motor wp) {
  moistureSensor = ms;
  ldrSensor = ls
  waterPump = wp
}

#endif

