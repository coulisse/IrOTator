

/* TODO:
 *  - set output power
 *  - sleep mode
 */

#include <ESP8266WiFi.h>
#include <Adafruit_HMC5883_U.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>


Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
sensors_event_t event;

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
  
          doCommand(command);
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


      Serial.println("HMC5883 Magnetometer Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!mag.begin())  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }

    /* Display some basic information on this sensor */
  displaySensorDetails();
}

void loop() {
  webSocket.loop();
  doCommand("getCompass");
  delay(500);  //TODO: eliminare
}

/*----------------------------------------------------
 parse command
----------------------------------------------------*/
void doCommand(const char* command){
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
  webSocket.broadcastTXT(buffer);
  //webSocket.sendTXT(channel,buffer);
  return;
}


int getCompass(){
    
  /* Get a new sensor event */ 
   
  mag.getEvent(&event);
 
  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 0.22;
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 
  
  Serial.print("Heading (degrees): "); Serial.println(headingDegrees);
  
  return headingDegrees;
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


void displaySensorDetails(void) {
  sensor_t sensor;
  mag.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}
