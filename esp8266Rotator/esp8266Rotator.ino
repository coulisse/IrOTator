 /******************************************************************************

                                 esp8266Rotator

    esp8266Rotator - part of IrOTator
    Copyright (C) 2017  Corrado Gerbaldo - IU1BOW


    this is the server side application.
    This application installed on the nodemcu will connect to WiFi networ and
    receive via websocket the command in order to pilotate the antenna rotator.


    mail: corrado.gerbaldo@gmail.com


 Changelog:
..............................................................................
 version: 1.0
 date...: 01/04/2017
 comment: first release

..............................................................................
  TODO:
  - CV / CCV only if websocket connection established
  - auto reboot
  - maual commands
  - set output power
  - sleep mode
  - send data only if websocket connected
   send data only if different previous data or after a specific time

******************************************************************************/

#define DEBUG 1

#include <ESP8266WiFi.h>
#include <Adafruit_HMC5883_U.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "rotator_commands.h"
#include "login.h"

Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified();
sensors_event_t event;
sensor_t sensor;

WebSocketsServer webSocket = WebSocketsServer(81);
const byte CCVPIN = 13;  //D7
const byte CVPIN = 15;   //D8
const byte LEDPIN = 12; //D6
const float TOLERANCE = 2.5;
float requestedDir = 999;
float currentDir = 999;

/*----------------------------------------------------------------------------*
 * WEBSOCKET event                                                            *
 *----------------------------------------------------------------------------*/
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
            #if  (DEBUG==Y)
               Serial.println(F("parseObject() failed"));
            #endif
            return;
        }
        const char* command   = root["c"];
        float value     = root["v"];
        const char* rc        = root["r"];

        doCommand(command,value);
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

/******************************************************************************
 * Setup                                                                      *
 ******************************************************************************/
void setup() {

   Serial.begin(9600);

   pinMode(LEDPIN,OUTPUT);
   pinMode(CCVPIN,OUTPUT);
   pinMode(CVPIN,OUTPUT);
   stopRotator();

   WiFi.begin(ssid, password);

    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.println();
    Serial.print(F("Local IP"));
    Serial.println(WiFi.localIP());
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

   blink();  //wifi ok - blink n. 1
    Serial.println("HMC5883 Magnetometer"); Serial.println("");
  
  /* Initialise the sensor */
  if(!mag.begin())  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }

    /* Display some basic information on this sensor */
  #if  (DEBUG == 1)
     displaySensorDetails();
  #endif

  blink();  //end setup - blink n. 2
}

/******************************************************************************
 * Main loop                                                                  *
 ******************************************************************************/
void loop() {


  webSocket.loop();
  doCommand(CMD_GET_COMPASS,0);
  //currentDir=getCompass();
  if (requestedDir  > currentDir + TOLERANCE) {
    Serial.print("RequestedDir: ");
    Serial.print(requestedDir);
    Serial.print(" CurrentDir: ");
    Serial.print(currentDir);
    Serial.print(" CurrentDir+TOL: ");
    Serial.println(currentDir+TOLERANCE);
    cv();
  } else if (requestedDir < currentDir - TOLERANCE) {
    Serial.print("RequestedDir: ");
    Serial.print(requestedDir);
    Serial.print(" CurrentDir: ");
    Serial.print(currentDir);
    Serial.print(" CurrentDir-TOL: ");
    Serial.println(currentDir-TOLERANCE);

    ccv();
  } else {
    stopRotator();
  }

  delay(500);  //TODO: eliminare
}
/*----------------------------------------------------------------------------*
 * Parse json command received by web                                         *
 * {"c":"getCompass","v":"valore","r":"return code"}                          *
 *----------------------------------------------------------------------------*/
void doCommand(const char* command, float value){

  
StaticJsonBuffer<200> jsonBuffer;

  char buffer[256];
  JsonObject& root = jsonBuffer.createObject();

  root["c"] = command;
  root["v"] = "command unknown";
  root["r"] = 99;

  if (strcmp(command,CMD_GET_COMPASS)==0) {
    root["v"] = getCompass();
    root["r"] = 0;
    currentDir = root["v"];
  } else {
    if (strcmp(command,CMD_SET_ROTATOR)==0){
      //root["v"] = setRotator(root["v"]);
      root["v"] = value;
      root["r"] = 0;
      requestedDir = (float) value;;
    } else {
      if (strcmp(command,CMD_STOP_ROTATOR)==0){
        root["v"] = stopRotator();
        root["r"] = 0;
          } else {
            if (strcmp(command,CMD_RESET)==0){
              root["v"] = reset();
              root["r"] = 0;
            }
  }}}

  root.printTo(buffer, sizeof(buffer));
  webSocket.broadcastTXT(buffer);
  //webSocket.sendTXT(channel,buffer);
  return;
}

/*-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*
 * Get the direction from the compass                                         *
 *-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
float getCompass(){

  static bool first_time = true;
  
  mag.getEvent(&event);
  if (first_time == true) {
    blink();  //first data from compass received - blink n. 3
    first_time = false;  
  }
  
  
  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(event.magnetic.y, event.magnetic.x);

  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
  float declinationAngle = 0.03595378;
  heading += declinationAngle;

  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;

  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;

  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI;
/*
if (headingDegrees < 90) {
  headingDegrees = headingDegrees * 2;
  };

if ((headingDegrees >=90) || (headingDegrees <270)) {
   headingDegrees=180+(headingDegrees-90)/2;
   };
//else already correct within degree or 2
*/

      /* Display some basic information on this sensor */
  #if  (DEBUG == 1)
    Serial.print("- Heading (rad-degrees): "); Serial.print (heading); Serial.print ("-"); Serial.println(headingDegrees);

  #endif

  return headingDegrees;
}

/*-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*
 * Set rotator position                                                       *
 *-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
int setRotator(int degree){
  Serial.print("Target in degrees: "+degree);
  //TODO: logica
  return degree;
}

/*-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*
 * Stop rotator                                                               *
 *-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
int stopRotator(){
  Serial.println("Stop Rotator");
  digitalWrite(CCVPIN, LOW); //changed to low, using transistors
  digitalWrite(CVPIN, LOW);  //changed to low, using transistors 
  return 0;
}
/*-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*
 * Counter Clock-wise rotation                                                *
 *-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
int ccv(){
  Serial.println("CCV");
  digitalWrite(CCVPIN, HIGH);
  digitalWrite(CVPIN, LOW);
  return 0;
}

/*-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*
 * Clock-wise rotation                                                        *
 *-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
int cv(){
  Serial.println("CV");
  digitalWrite(CCVPIN, LOW);
  digitalWrite(CVPIN, HIGH);
  return 0;
}

/*-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*
 * Reset and reboot the system                                                *
 *-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
int reset(){
  Serial.println(F("Stop rotator..."));
  doCommand(CMD_STOP_ROTATOR,0);
  Serial.println(F("Disconnecting websocket..."));
  webSocket.disconnect();
  Serial.println(F("Ending wifi..."));
  WiFi.disconnect();
  Serial.println(F("Rebooting..."));
  Serial.end();
  ESP.reset();
}

/*-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*
 * Display basic information about the hmc5883l sensor                        *
 *-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
void displaySensorDetails(void) {
  //sensor_t sensor;
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

/*-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*
 * Turn on - off the control led                                              *
 *-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
void blink(void) {
  digitalWrite(LEDPIN, LOW);  
  digitalWrite(LEDPIN, HIGH);
  delay(1000);  
  digitalWrite(LEDPIN, LOW);
  delay(1000);   
}

