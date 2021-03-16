// PMS5003   NodeMCU
//   VCC        Vin       +5V
//   GND        GND        0V
//   TX         RX0        
//Disconnect TX while programming via USB 
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

// Replace with your network credentials
const char* ssid = "SKYPEMHG";
const char* password = "8NHetSWQAJ75";

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};
struct pms5003data data;
AsyncWebServer server(80);
AsyncEventSource events("/events");
JSONVar readings;
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

String getSensorReadings(){
  readings["PMS10"] = data.pm10_standard;
  readings["PMS25"] =  data.pm25_standard;
  readings["PMS100"] = data.pm100_standard;
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
boolean readPMSdata() {
  if (!Serial.available()) {
    return false;
  }
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (Serial.peek() != 0x42) {
    Serial.read();
    return false;
  }
  // Now read all 32 bytes
  if (Serial.available() < 32) {
    return false;
  }    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  Serial.readBytes(buffer, 32);
  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }
  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
  memcpy((void *)&data, (void *)buffer_u16, 30);
  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  return true;
}
void setup() {
  Serial.begin(9600); //9600 for PMS5003
  initWiFi();
  LittleFS.begin();
  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });
  server.serveStatic("/", LittleFS, "/");
  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
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
  if ((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 30 seconds
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    lastTime = millis();
  }
  if(readPMSdata()){
    //Serial.print(data.pm10_standard); Serial.print(":");
    //Serial.print(data.pm25_standard); Serial.print(":");
    //Serial.print(data.pm100_standard); Serial.println(":");
  }
}