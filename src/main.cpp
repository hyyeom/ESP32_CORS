#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include <Adafruit_AM2320.h>
#include "SPIFFS.h"

const char* ssid = "EJD";
const char* password = "5165135199ejd";

IPAddress ip(192, 168, 1, 102);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);

const int ledPin = 12;
const int dataLED = 27;
const int wifiLED = 25;
String ledState;
JSONVar readings;

AsyncWebServer server(80);
AsyncEventSource events("/events");

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if(!WiFi.config(ip, gateway, subnet, dns)){
    Serial.println("Static IP Configuration Failed");
  }
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  digitalWrite(wifiLED, HIGH);
  Serial.println(WiFi.localIP());
}
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}
Adafruit_AM2320 AM2320 = Adafruit_AM2320();
void initAM2320(){
  AM2320.begin();
}
void sLED(){
  for(int i = 0; i < 3; i++){
    digitalWrite(dataLED, HIGH);
    delay(100);
    digitalWrite(dataLED, LOW);
    delay(100);
  }
}
String myState(){
  sLED();
  if(digitalRead(ledPin)) {
      ledState = "OFF";
    }
    else {
      ledState = "ON";
    }
    return ledState;
}
String getSensorReadings(){
  readings["temperature"] = String(AM2320.readTemperature());
  readings["humidity"] = String(AM2320.readHumidity());
  //readings["battary"] = String(analogRead(A0)/100.0F);
  readings["sw"] = myState();
  //readings["key"] = String(myKey);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}
void setup() {
  Serial.begin(9600);
  initWiFi();
  initSPIFFS();
  initAM2320();
  pinMode(ledPin , OUTPUT);
  pinMode(wifiLED , OUTPUT);
  pinMode(dataLED , OUTPUT);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", "text/html");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("/Data");
    String json = getSensorReadings();
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
    json = String();
  });

  server.on("/onoff", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("/onoff");
    if(digitalRead(ledPin)) {
      digitalWrite(ledPin, LOW);
    }
    else {
      digitalWrite(ledPin, HIGH);
    }
    String json = getSensorReadings();
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
    json = String();
  });
  events.onConnect([](AsyncEventSourceClient *client){
  if(client->lastId()){
    Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
  }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  // Start server
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
}




