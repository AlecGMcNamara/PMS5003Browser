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

const char* ssid = "SKYPEMHG";
const char* password = "8NHetSWQAJ75";

#define MAX_DATA_FILES 80    //0-79

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
FSInfo fs_info;
int intFileNumber =0;
bool blRedraw = false;

String getFileName(int tempFileNumber){
  char tempFileName[10];
  sprintf(tempFileName,"/data.%03d", tempFileNumber);
  return tempFileName;
}

unsigned long FreeDiskSpace(){
  LittleFS.info(fs_info);
  return fs_info.totalBytes - fs_info.usedBytes;
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

void ChartRedraw(){
  File fileRead;
  String fileName;
  static int RedrawFile = 0;
  static bool ReDrawing = false;
  unsigned long tempRecordTime=0;
  StaticJsonDocument <100> jsonRedraw;
  String strJsonReDraw;
  // redraw requested, by browser, send all recorded data
  if(blRedraw && ws.availableForWriteAll()){ 
      if(!ReDrawing){
        ReDrawing=true;
        RedrawFile = intFileNumber;   
      }  
      fileName = getFileName(RedrawFile);
      fileRead = LittleFS.open(fileName, "r");
      if (fileRead){
        strJsonReDraw = fileRead.readString();
      //  Serial.print("Reading File:");    Serial.println(strJsonReDraw);
        deserializeJson(jsonRedraw,strJsonReDraw);
        tempRecordTime =jsonRedraw["ReadingTime"];
        jsonRedraw["ReadingTime"] = long(tempRecordTime - millis());
        strJsonReDraw="";// clear string or serialize function appends!!!
        serializeJson(jsonRedraw,strJsonReDraw);
        ws.textAll(strJsonReDraw);
        //Serial.print("Redrawing:"); Serial.println(strJsonReDraw);
        fileRead.close();
      }
      RedrawFile ++;
      if(RedrawFile==MAX_DATA_FILES) RedrawFile=0;
      if (RedrawFile == intFileNumber){
        ReDrawing = false;
        blRedraw = false;
      }
  }
}
void sendMessage()
{
  static unsigned long nextSendDue = 0;
  if (!blRedraw && nextSendDue < millis() && ws.availableForWriteAll())
  {
    StaticJsonDocument<100> jsonSend;
    jsonSend["ReadingTime"] = 0;
    jsonSend["PMS10"] = dataP.pm10_standard;
    jsonSend["PMS25"] = dataP.pm25_standard;
    jsonSend["PMS100"] = dataP.pm100_standard;

    String strJsonSend;
    serializeJson(jsonSend, strJsonSend);
   // Serial.print("Sending Message:");    Serial.println(strJsonSend);
    ws.textAll(strJsonSend);
    //write data file to flash filename /data.000-/data.099
    jsonSend["ReadingTime"] = long(millis());
    strJsonSend = ""; //clear string of next json string is appended!!!!
    serializeJson(jsonSend, strJsonSend);
    File fileWrite;
    String fileName = getFileName(intFileNumber);
    if (fileWrite = LittleFS.open(fileName, "w"))
    {
     // Serial.print("Writing File:");    Serial.println(strJsonSend);
      fileWrite.println(strJsonSend);
      fileWrite.close();
      intFileNumber++;
      nextSendDue = millis()+10000;
      if (intFileNumber >= MAX_DATA_FILES)
        intFileNumber = 0;
    }
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {   
      // browser not sending any messages
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
 switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      blRedraw = true; //browser refreshed, redraw all chart
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

void setup() {
  Serial.begin(9600); //9600 for PMS5003
  initWiFi();
  LittleFS.begin();
  Serial.printf("File system free space :%lu bytes\n", FreeDiskSpace());

  //delete all data files /data.000 to /data.079
  Serial.print("Deleting old data.....");
  for (intFileNumber=0;intFileNumber < MAX_DATA_FILES;intFileNumber++){
      LittleFS.remove(getFileName(intFileNumber));
  }
  intFileNumber = 0;
  Serial.println("done.");
  
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });
  server.serveStatic("/", LittleFS, "/");
  server.begin();
}
void loop() {
  readPMSdata();
  sendMessage();
  ChartRedraw(); 
  ws.cleanupClients();
}