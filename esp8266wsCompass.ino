/* TODO:
 *  - set output power
 *  - sleep mode
 */


#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

WebSocketsServer webSocket = WebSocketsServer(81);
const char* ssid     ="hyperline-11635";
const char* password = "auj6xai6iN";


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
StaticJsonBuffer<200> jsonBuffer;

      switch(type) {
        
        case WStype_DISCONNECTED:
            break;
            
        case WStype_CONNECTED: {
              IPAddress ip = webSocket.remoteIP(num);  
            }
            break;
            
        case WStype_TEXT: {
          String text = String((char *) &payload[0]);
          JsonObject& root = jsonBuffer.parseObject(text);
          if (!root.success()) {
              Serial.println(F("parseObject() failed"));
              return;
          }
  
          const char* command   = root["c"];
          const char* value     = root["v"];
          const char* rc        = root["r"];
  
          doCommand(command,num);
      }

     break;
/*        
    case WStype_BIN:
 
        hexdump(payload, lenght);

        // echo data back to browser
        webSocket.sendBIN(num, payload, lenght);
        break; */
    }
}

void setup() {
   
   Serial.begin(115200);    
   WiFi.begin(ssid, password);

    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.print(F("Local IP"));
    Serial.println(WiFi.localIP());
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();
}

/*----------------------------------------------------
 parse command
----------------------------------------------------*/
void doCommand(const char* command, uint8_t channel){
//{"c":"getCompass","v":"valore","r":"return code"}
StaticJsonBuffer<200> jsonBuffer;

  char buffer[256];
  JsonObject& root = jsonBuffer.createObject();

  root["c"] = command;
  root["v"] = "command unknown";
  root["r"] = 99;

  if (strcmp(command,"getCompass")==0) {
    root["v"] = getCompass();
    root["r"] = 0;
  } else {
    if (strcmp(command,"setCompass")==0){
      root["v"] = setCompass(2);
      root["r"] = 0;
    } else {
      if (strcmp(command,"stopRotator")==0){
        root["v"] = stopRotator();
        root["r"] = 0;
      } else {
        if (strcmp(command,"leftRotator")==0){
          root["v"] = leftRotator();
          root["r"] = 0;
        } else {
          if (strcmp(command,"rightRotator")==0){
            root["v"] = rightRotator();
            root["r"] = 0;
          } else {
            if (strcmp(command,"standBy")==0){
              root["v"] = standBy();
              root["r"] = 0;
            } else {
              if (strcmp(command,"reset")==0){
                root["v"] = standBy();
                root["r"] = 0;
              }
  }}}}}}

  root.printTo(buffer, sizeof(buffer));
  //webSocket.broadcastTXT(buffer);
  webSocket.sendTXT(channel,buffer);
  return;
}


int getCompass(){
  return random(0, 359);
}

int setCompass(int degree){
  return 2;
}

int stopRotator(){
  return 3;
}

int leftRotator(){
  return 4;
}

int rightRotator(){
  return 5;
}

int standBy(){
  return 6;
}

int reset(){
  return 7;
}

