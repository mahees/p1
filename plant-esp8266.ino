#include "WiFiUtil.h"
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <MoistureSensor.h>
#include <Motor.h>
#include <DigitalPin.h>
#include <Logger.h>
#include "AnalogReader.h"
#include "utility.h"
#include "conf.h"
#include "WebServerHandlers.h"

#define msReadSensorDelay 1200000 //20min
#define msWiFiCheckDelay 60000
#define msBuzzorBeforeWateringDelay 100
#define msWateringActiveDelay 2500
#define msRetryAfterWateringDelay 30000
#define moistLevel 7000
#define msLDRCheckSensorDelay 300000 //5 min

unsigned long lastTakeAndProcessMillis = millis();
unsigned long lastWiFiCheckMillis = millis();
unsigned long lastLDRCheckMillis = millis();

MoistureSensor moistureSensor(A0, 12);
AnalogReader ldrSensor(A0, 14);
Motor waterPump(13, false);
//DigitalPin buzzor(2);
//Logger logger(9600);

ESP8266WebServer server(54123);


void setup(void) {
  Serial.begin(115200);

  checkOrconnectToWiFi();

  attachWebServerHandlers(server);

  injectWebServerHandlerDependencies(moistureSensor, ldrSensor, waterPump);

  server.begin();
  Serial.println("HTTP server started");

  Serial.flush();
}

void loop(void) {

  checkDelayAndRun(lastWiFiCheckMillis, msWiFiCheckDelay, checkOrconnectToWiFi);
  checkDelayAndRun(lastLDRCheckMillis, msLDRCheckSensorDelay, checkLDR);
  checkDelayAndRun(lastTakeAndProcessMillis, msReadSensorDelay, takeAndprocessReading);

  server.handleClient();
}


void checkLDR() {
  Serial.println("taking LDR reading: ");
  int reading = ldrSensor.takeReading(2, 100);
  Serial.println(reading);

  postDataToAPI("field2", reading);
}


int getMoistureReading() {
  Serial.println("taking reading: ");
  int moistureReading = moistureSensor.takeReading();
  Serial.println(moistureReading);

  postDataToAPI("field1", moistureReading);

  return moistureReading;
}


void postDataToAPI(String field, int reading) {

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection to host api site failed");
    return;
  }

  // We now create a URI for the request
  String url = "/update/";
  url += "?api_key=";
  url += apiKey;
  url += "&";
  url += field;
  url += "=";
  url += reading;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);

  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  client.flush();

  Serial.println();
  Serial.println("closing connection");
  Serial.flush();
}


void takeAndprocessReading() {
  int moistureReading = getMoistureReading();

  if (moistureReading < moistLevel)
  {
    Serial.println("soil moist");
    return;
  }

  Serial.println("soil dry: activating pump");
  //buzzor.toggle(msBuzzorBeforeWateringDelay);
  waterPump.activate(msWateringActiveDelay);
  Serial.println("reading in a few secs");
  delay(msRetryAfterWateringDelay);
  takeAndprocessReading();
}
