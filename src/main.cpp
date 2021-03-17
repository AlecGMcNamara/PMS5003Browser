// PMS5003   NodeMCU
//   VCC        Vin (+5V)
//   GND        GND
//   TX         RX0        
//Disconnect TX while programming via USB 
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJSON.h>
#include <time.h>

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
struct pms5003data dataP;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
unsigned long updateTime = 0;
FSInfo fs_info;

void sendMessage()
  {
      StaticJsonDocument<100> jsonSend;
      //set up message from IO and send to browser(s)
      jsonSend["PMS10"] = dataP.pm10_standard;
      jsonSend["PMS25"] =  dataP.pm25_standard;
      jsonSend["PMS100"] = dataP.pm100_standard;
      String jsonString;
      serializeJson(jsonSend,jsonString);
      ws.textAll(jsonString);
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
  if (Serial.peek() != 0x42) {
    Serial.read();
    return false;
  }
  if (Serial.available() < 32) {
    return false;
  }    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  Serial.readBytes(buffer, 32);
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
  memcpy((void *)&dataP, (void *)buffer_u16, 30);
  if (sum != dataP.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  return true;
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) { 
    data[len] = 0;
    StaticJsonDocument<100> jsonReceived;
    deserializeJson(jsonReceived,(char*)data);
     // add readings to json object 
    jsonReceived["PMS10"] = dataP.pm10_standard;
    jsonReceived["PMS25"] =  dataP.pm25_standard;
    jsonReceived["PMS100"] = dataP.pm100_standard;
    String jsonString;
    serializeJson(jsonReceived,jsonString);
    ws.textAll(jsonString);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
 switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      //sendMessage();
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
    break;
  }
}

unsigned long FreeDiskSpace(){
  LittleFS.info(fs_info);
  return fs_info.totalBytes - fs_info.usedBytes;
}

void setup() {
  Serial.begin(9600); //9600 for PMS5003
  initWiFi();
  LittleFS.begin();
  Serial.printf("File system free space :%lu bytes\n", FreeDiskSpace());

  /*File fileTest;

  LittleFS.remove("/data.txt");

  if(!LittleFS.exists("/data.txt")) {
      Serial.println("Data file missing");
      if(fileTest = LittleFS.open("/data.txt","w")) { 
        Serial.println("Created data file");
        fileTest.println("This is a write to file test.");
        fileTest.close();
      }
  }
  fileTest = LittleFS.open("/data.txt", "r");
    if (fileTest){
        Serial.print("Opened file...read...");
        Serial.println(fileTest.readString());
        fileTest.close();
    }

  */

  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });
  server.serveStatic("/", LittleFS, "/");
  server.begin();
}
void loop() {
  //if (updateTime < millis()) {
  //    sendMessage();
  //    updateTime += 10000;  //send update every 30 seconds
  //}
  readPMSdata();
  ws.cleanupClients();
}