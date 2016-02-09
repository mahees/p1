#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <MoistureSensor.h>
#include <Motor.h>
#include <DigitalPin.h>
#include <Logger.h>
#include "conf.h"


#define msReadSensorDelay 1200000
#define msWiFiCheckDelay 60000
#define msBuzzorBeforeWateringDelay 100
#define msWateringActiveDelay 2500
#define msRetryAfterWateringDelay 30000
#define moistLevel 7000

unsigned long lastTakeAndProcessMillis = millis();
unsigned long lastWiFiCheckMillis = millis();


MoistureSensor moistureSensor(A0, 12);
Motor waterPump(13, false);
//DigitalPin buzzor(2);
//Logger logger(9600);

ESP8266WebServer server(54123);

void checkOrconnectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Connection failed");
      return;
    }
    delay(500);
    Serial.print(".");
  }
}


void setup(void) {
  Serial.begin(115200);

  checkOrconnectToWiFi();

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/take-reading", getReadingHandler);
  server.on("/water-plant", waterPlantHandler);
  server.on("/take-and-process-reading", takeAndprocessReadingHandler);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  Serial.flush();
}

void loop(void) {
  unsigned long currentTimeMillis = millis();
  
  if (currentTimeMillis - lastWiFiCheckMillis >= msWiFiCheckDelay) {
    lastWiFiCheckMillis = currentTimeMillis;
    checkOrconnectToWiFi();
  }
  
  if (currentTimeMillis - lastTakeAndProcessMillis >= msReadSensorDelay) {
    lastTakeAndProcessMillis = currentTimeMillis;
    takeAndprocessReading();
  }
  server.handleClient();
}

void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}

void getReadingHandler() {
  Serial.println("taking reading: ");
  int moistureReading = moistureSensor.takeReading();
  Serial.println(moistureReading);

  postMoistureDataToAPI(moistureReading);

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


void postMoistureDataToAPI(int moistureReading) {

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
  url += "&field1=";
  url += moistureReading;

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

  Serial.println("taking reading: ");
  int moistureReading = moistureSensor.takeReading();
  Serial.println(moistureReading);

  postMoistureDataToAPI(moistureReading);

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
